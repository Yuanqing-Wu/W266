#include "Buffer.h"
#include "Unit.h"

template<typename T>
void AreaBuf<T>::fill(const T &val) {
    if(T(0) == val) {
        if(width == stride) {
            ::memset(buf, 0, width * height * sizeof(T));
        } else {
            T* dest = buf;
            size_t line = width * sizeof(T);
            for(unsigned y = 0; y < height; y++) {
                ::memset(dest, 0, line);
                dest += stride;
            }
        }
    } else {
        if(width == stride) {
            std::fill_n(buf, width * height, val);
        } else {
            T* dest = buf;
            for(int y = 0; y < height; y++, dest += stride) {
                std::fill_n(dest, width, val);
            }
        }
    }
}

template<typename T>
void AreaBuf<T>::memset(const int val) {
    if(width == stride) {
        ::memset(buf, val, width * height * sizeof(T));
    } else {
        T* dest = buf;
        size_t line = width * sizeof(T);
        for(int y = 0; y < height; y++) {
            ::memset(dest, val, line);
            dest += stride;
        }
    }
}

template<typename T>
void AreaBuf<T>::copyFrom(const AreaBuf<const T> &other) {
    CHECK_FATAL(width  != other.width,  "Incompatible size");
    CHECK_FATAL(height != other.height, "Incompatible size");

    if(buf == other.buf) {
        return;
    }

    if(ptrdiff_t(width) == stride && stride == other.stride) {
        memcpy(buf, other.buf, width * height * sizeof(T));
    } else {
        T* dst                     =       buf;
        const T* src               = other.buf;
        const ptrdiff_t srcStride = other.stride;

        for(unsigned y = 0; y < height; y++) {
            memcpy(dst, src, width * sizeof(T));

            dst += stride;
            src += srcStride;
        }
    }
}


template<typename T>
void AreaBuf<T>::reconstruct(const AreaBuf<const T> &pred, const AreaBuf<const T> &resi, const ClpRng& clp_rng) {
    THROW_FATAL("Type not supported");
}

template<>
void AreaBuf<Pel>::reconstruct(const AreaBuf<const Pel> &pred, const AreaBuf<const Pel> &resi, const ClpRng& clp_rng) {
    const Pel* src1 = pred.buf;
    const Pel* src2 = resi.buf;
          Pel* dest =      buf;

    const ptrdiff_t src1_stride = pred.stride;
    const ptrdiff_t src2_stride = resi.stride;
    const ptrdiff_t dst_stride  =      stride;

#define RECO_OP(ADDR) dest[ADDR] = clipPel(src1[ADDR] + src2[ADDR], clp_rng)
#define RECO_INC         \
    src1 += src1_stride; \
    src2 += src2_stride; \
    dest += dst_stride;  \

    SIZE_AWARE_PER_EL_OP(RECO_OP, RECO_INC);

#undef RECO_OP
#undef RECO_INC
}

template<typename T>
void AreaBuf<T>::subtract(const AreaBuf<const T> &other) {
    CHECK_FATAL(width  != other.width,  "Incompatible size");
    CHECK_FATAL(height != other.height, "Incompatible size");

          T* dest =       buf;
    const T* subs = other.buf;

#define SUBS_INC      \
    dest +=       stride; \
    subs += other.stride; \

#define SUBS_OP(ADDR) dest[ADDR] -= subs[ADDR]

    SIZE_AWARE_PER_EL_OP(SUBS_OP, SUBS_INC);

#undef SUBS_OP
#undef SUBS_INC
}

template<typename T>
void AreaBuf<T>::extendBorderPel(unsigned margin) {
    T*        p = buf;
    int       h = height;
    int       w = width;
    ptrdiff_t s = stride;

    CHECK_FATAL((w + 2 * margin) > s, "Size of buffer too small to extend");
    // do left and right margins
    for(int y = 0; y < h; y++) {
        for(int x = 0; x < margin; x++) {
            *(p - margin + x) = p[0];
            p[w + x]            = p[w - 1];
        }
        p += s;
    }

    // p is now the (0,height) (bottom left of image within bigger picture
    p -= (s + margin);
    // p is now the (-margin, height-1)
    for(int y = 0; y < margin; y++) {
        ::memcpy(p + (y + 1) * s, p, sizeof(T) * (w + (margin << 1)));
    }

    // pi is still (-marginX, height-1)
    p -= ((h - 1) * s);
    // pi is now (-marginX, 0)
    for(int y = 0; y < margin; y++) {
        ::memcpy(p - (y + 1) * s, p, sizeof(T) * (w + (margin << 1)));
    }
}

template<typename T>
void AreaBuf<T>::extendBorderPel(unsigned margin, bool left, bool right, bool top, bool bottom) {
    CHECK_FATAL((width + left*margin + right*margin) > stride, "Size of buffer too small to extend");
    // do left and right margins

    if(left && right) {
        T* p = buf;
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < margin; x++) {
                p[-(int)margin + x] = p[0];
                p[width + x]   = p[width - 1];
            }
            p += stride;
        }
    } else if(left) {
        T* p = buf;
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < margin; x++) {
                p[-(int)margin + x] = p[0];
            }
            p += stride;
        }
    } else if(right) {
        T* p = buf;
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < margin; x++) {
                p[width + x] = p[width - 1];
            }
            p += stride;
        }
    }

    const int copylen = width + (left ? margin : 0) + (right ? margin : 0);
    if(bottom) {
        T* p = buf + stride * height;
        if(left)
            p -= margin;

        // p is now the (-margin, height)
        for(int y = 0; y < margin; y++) {
            ::memcpy(p + y * stride, p - stride, sizeof(T) * copylen);
        }
    }

    if(top) {
        T* p = buf;
        if(left)
            p -= margin;

        // pi is now (-marginX, 0)
        for(int y = -(int)margin; y < 0; y++) {
            ::memcpy(p + y * stride, p, sizeof(T) * copylen);
        }
    }
}

template<typename T>
void AreaBuf<T>::padBorderPel(unsigned marginX, unsigned marginY, int dir) {
    T*   p = buf;
    auto s = stride;
    int  h = height;
    int  w = width;

    CHECK_FATAL(w  > s, "Size of buffer too small to extend");

    // top-left margin
    if (dir == 1) {
        for(int y = 0; y < marginY; y++) {
            for(int x = 0; x < marginX; x++) {
                p[x] = p[marginX];
            }
            p += s;
        }
    }

    // bottom-right margin
    if (dir == 2) {
        p = buf + s * (h - marginY) + w - marginX;

        for(int y = 0; y < marginY; y++) {
            for(int x = 0; x < marginX; x++) {
                p[x] = p[-1];
            }
            p += s;
        }
    }
}

template<typename T>
void UnitBuf<T>::fill(const T &val) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].fill(val);
    }
}

template<typename T>
void UnitBuf<T>::copyFrom(const UnitBuf<const T> &other) {
    CHECK_FATAL(chromaFormat != other.chromaFormat, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].copyFrom(other.bufs[i]);
    }
}

template<typename T>
void UnitBuf<T>::subtract(const UnitBuf<const T> &other) {
    CHECK_FATAL(chromaFormat != other.chromaFormat, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].subtract(other.bufs[i]);
    }
}

template<typename T>
void UnitBuf<T>::reconstruct(const UnitBuf<const T> &pred, const UnitBuf<const T> &resi, const ClpRng& clp_rngs) {
    CHECK_FATAL(chromaFormat != pred.chromaFormat, "Incompatible formats");
    CHECK_FATAL(chromaFormat != resi.chromaFormat, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].reconstruct(pred.bufs[i], resi.bufs[i], clp_rngs);
    }
}

template<typename T>
void UnitBuf<T>::extendBorderPel(unsigned margin) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].extendBorderPel(margin);
    }
}

template<typename T>
void UnitBuf<T>::extendBorderPel(unsigned margin, bool left, bool right, bool top, bool bottom) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].extendBorderPel(margin, left, right, top, bottom);
    }
}

template<typename T>
void UnitBuf<T>::padBorderPel(unsigned margin, int dir) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].padBorderPel(margin >> getComponentScaleX(ComponentID(i), chromaFormat), margin >> getComponentScaleY(ComponentID(i), chromaFormat), dir);
    }
}

template<typename T>
UnitBuf<T> UnitBuf<T>::subBuf(const UnitArea& subArea) {
  UnitBuf<T> subBuf;
  subBuf.chromaFormat = chromaFormat;
  unsigned block_idx = 0;

  for(auto &subAreaBuf : bufs) {
    subBuf.bufs.push_back(subAreaBuf.subBuf(subArea.blocks[block_idx].pos(), subArea.blocks[block_idx].size()));
    block_idx++;
  }

  return subBuf;
}


template<typename T>
const UnitBuf<const T> UnitBuf<T>::subBuf(const UnitArea& subArea) const {
  UnitBuf<const T> subBuf;
  subBuf.chromaFormat = chromaFormat;
  unsigned block_idx = 0;

  for(const auto &subAreaBuf : bufs) {
    subBuf.bufs.push_back(subAreaBuf.subBuf(subArea.blocks[block_idx].pos(), subArea.blocks[block_idx].size()));
    block_idx++;
  }

  return subBuf;
}

template<typename T>
UnitBuf<T> UnitBuf<T>::subBuf(const Area & subArea) {
  UnitBuf<T> subBuf;
  subBuf.chromaFormat = chromaFormat;
  unsigned block_idx = 0;

  for(auto &subAreaBuf : bufs) {
    const int scaleX = getComponentScaleX(ComponentID(block_idx), chromaFormat);
    const int scaleY = getComponentScaleY(ComponentID(block_idx), chromaFormat);
    const Area scaledArea(subArea.pos().x >> scaleX, subArea.pos().y >> scaleY, subArea.size().width >> scaleX, subArea.size().height >> scaleY);
    subBuf.bufs.push_back(subAreaBuf.subBuf(scaledArea.pos(), scaledArea.size()));
    block_idx++;
  }

  return subBuf;
}

template<typename T>
const UnitBuf<const T> UnitBuf<T>::subBuf(const Area & subArea) const {
  UnitBuf<T> subBuf;
  subBuf.chromaFormat = chromaFormat;
  unsigned block_idx = 0;

  for(auto &subAreaBuf : bufs) {
    const int scaleX = getComponentScaleX(ComponentID(block_idx), chromaFormat);
    const int scaleY = getComponentScaleY(ComponentID(block_idx), chromaFormat);
    const Area scaledArea(subArea.pos().x >> scaleX, subArea.pos().y >> scaleY, subArea.size().width >> scaleX, subArea.size().height >> scaleY);
    subBuf.bufs.push_back(subAreaBuf.subBuf(scaledArea.pos(), scaledArea.size()));
    block_idx++;
  }

  return subBuf;
}

PelStorage::PelStorage() {
    for(uint32_t i = 0; i < MAX_NUM_COMPONENT; i++)
    {
        m_Origin[i]        = nullptr;
    }
}

PelStorage::~PelStorage() {
    destroy();
}

void PelStorage::swap(PelStorage& other) {
    const uint32_t numCh = getNumberValidComponents(chromaFormat);

    for(uint32_t i = 0; i < numCh; i++) {
        // check this otherwise it would turn out to get very weird
        CHECK_FATAL(chromaFormat              != other.chromaFormat             , "Incompatible formats");
        CHECK_FATAL(get(ComponentID(i))        != other.get(ComponentID(i))       , "Incompatible formats");
        CHECK_FATAL(get(ComponentID(i)).stride != other.get(ComponentID(i)).stride, "Incompatible formats");

        std::swap(bufs[i].buf,    other.bufs[i].buf);
        std::swap(bufs[i].stride, other.bufs[i].stride);
        std::swap(m_Origin[i],    other.m_Origin[i]);
    }
}

void PelStorage::createFromBuf(PelUnitBuf buf) {
    chromaFormat = buf.chromaFormat;

    const uint32_t numCh = getNumberValidComponents(chromaFormat);

    bufs.resize(numCh);

    for(uint32_t i = 0; i < numCh; i++) {
        PelBuf cPelBuf = buf.get(ComponentID(i));
        bufs[i] = PelBuf(cPelBuf.bufAt(0, 0), cPelBuf.stride, cPelBuf.width, cPelBuf.height);
    }
}

void PelStorage::create(const UnitArea &_UnitArea) {
    create(_UnitArea.chromaFormat, _UnitArea.blocks[0]);
}

void PelStorage::create(const ChromaFormat _chromaFormat, const Size& _size, const unsigned _maxCUSize, const unsigned _margin, const unsigned _alignmentByte, const bool _scaleChromaMargin) {
    CHECK_FATAL(!bufs.empty(), "Trying to re-create an already initialized buffer");

    chromaFormat = _chromaFormat;

    const uint32_t numCh = getNumberValidComponents(_chromaFormat);

    unsigned extHeight = _size.height;
    unsigned extWidth  = _size.width;

    if(_maxCUSize) {
        extHeight = ((_size.height + _maxCUSize - 1) / _maxCUSize) * _maxCUSize;
        extWidth  = ((_size.width  + _maxCUSize - 1) / _maxCUSize) * _maxCUSize;
    }

    const unsigned _alignment = _alignmentByte / sizeof(Pel);

    for(uint32_t i = 0; i < numCh; i++) {
        const ComponentID compId = ComponentID(i);
        const unsigned scaleX = getComponentScaleX(compId, _chromaFormat);
        const unsigned scaleY = getComponentScaleY(compId, _chromaFormat);

        unsigned scaledHeight = extHeight >> scaleY ;
        unsigned scaledWidth  = extWidth  >> scaleX;
        unsigned ymargin      = _margin >> (_scaleChromaMargin ? scaleY : 0);
        unsigned xmargin      = _margin >> (_scaleChromaMargin ? scaleX : 0);

        if(_alignment && xmargin) {
            xmargin = ((xmargin + _alignment - 1) / _alignment) * _alignment;
        }

        SizeType totalWidth   = scaledWidth + 2 * xmargin;
        SizeType totalHeight  = scaledHeight +2 * ymargin;

        if(_alignment) {
            // make sure buffer lines are align
            CHECK_FATAL(_alignmentByte != MEMORY_ALIGN_DEF_SIZE, "Unsupported alignment");
            totalWidth = ((totalWidth + _alignment - 1) / _alignment) * _alignment;
        }

        uint32_t area = totalWidth * totalHeight;
        CHECK_FATAL(!area, "Trying to create a buffer with zero area");

        m_OrigSize[i] = Size{ totalWidth, totalHeight };
        m_Origin[i] = (Pel*) xMalloc(Pel, area);
        Pel* topLeft = m_Origin[i] + totalWidth * ymargin + xmargin;
        bufs.push_back(PelBuf(topLeft, totalWidth, _size.width >> scaleX, _size.height >> scaleY));
    }
}

void PelStorage::destroy() {
    chromaFormat = NUM_CHROMA_FORMAT;
    for(uint32_t i = 0; i < MAX_NUM_COMPONENT; i++) {
        if(m_Origin[i]) {
            m_Origin[i] = nullptr;
        }
    }
    bufs.clear();
}

PelBuf PelStorage::getBuf(const ComponentID compId) {
    return bufs[compId];
}

const CPelBuf PelStorage::getBuf(const ComponentID compId) const {
    return bufs[compId];
}

PelBuf PelStorage::getBuf(const CompArea &blk) {
    const PelBuf& r = bufs[blk.compId()];

    CHECKD(rsAddr(blk.bottomRight(), r.stride) >= ((r.height - 1) * r.stride + r.width), "Trying to access a buf outside of bound!");

    return PelBuf(r.buf + rsAddr(blk, r.stride), r.stride, blk);
}

const CPelBuf PelStorage::getBuf(const CompArea &blk) const {
    const PelBuf& r = bufs[blk.compId()];
    return CPelBuf(r.buf + rsAddr(blk, r.stride), r.stride, blk);
}

PelUnitBuf PelStorage::getBuf(const UnitArea &unit) {
    return (chromaFormat == CHROMA_400) ? PelUnitBuf(chromaFormat, getBuf(unit.Y())) : PelUnitBuf(chromaFormat, getBuf(unit.Y()), getBuf(unit.Cb()), getBuf(unit.Cr()));
}

const CPelUnitBuf PelStorage::getBuf(const UnitArea &unit) const {
    return (chromaFormat == CHROMA_400) ? CPelUnitBuf(chromaFormat, getBuf(unit.Y())) : CPelUnitBuf(chromaFormat, getBuf(unit.Y()), getBuf(unit.Cb()), getBuf(unit.Cr()));
}