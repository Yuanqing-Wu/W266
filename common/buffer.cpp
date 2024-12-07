#include "buffer.h"

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
void AreaBuf<T>::copy_from(const AreaBuf<const T> &other) {
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
        const ptrdiff_t src_stride = other.stride;

        for(unsigned y = 0; y < height; y++) {
            memcpy(dst, src, width * sizeof(T));

            dst += stride;
            src += src_stride;
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

#define RECO_OP(ADDR) dest[ADDR] = clip_pel(src1[ADDR] + src2[ADDR], clp_rng)
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
void AreaBuf<T>::extend_border_pel(unsigned margin) {
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

    // pi is still (-margin_x, height-1)
    p -= ((h - 1) * s);
    // pi is now (-margin_x, 0)
    for(int y = 0; y < margin; y++) {
        ::memcpy(p - (y + 1) * s, p, sizeof(T) * (w + (margin << 1)));
    }
}

template<typename T>
void AreaBuf<T>::extend_border_pel(unsigned margin, bool left, bool right, bool top, bool bottom) {
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

        // pi is now (-margin_x, 0)
        for(int y = -(int)margin; y < 0; y++) {
            ::memcpy(p + y * stride, p, sizeof(T) * copylen);
        }
    }
}

template<typename T>
void AreaBuf<T>::pad_border_pel(unsigned margin_x, unsigned margin_y, int dir) {
    T*   p = buf;
    auto s = stride;
    int  h = height;
    int  w = width;

    CHECK_FATAL(w  > s, "Size of buffer too small to extend");

    // top-left margin
    if (dir == 1) {
        for(int y = 0; y < margin_y; y++) {
            for(int x = 0; x < margin_x; x++) {
                p[x] = p[margin_x];
            }
            p += s;
        }
    }

    // bottom-right margin
    if (dir == 2) {
        p = buf + s * (h - margin_y) + w - margin_x;

        for(int y = 0; y < margin_y; y++) {
            for(int x = 0; x < margin_x; x++) {
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
void UnitBuf<T>::copy_from(const UnitBuf<const T> &other) {
    CHECK_FATAL(chroma_format != other.chroma_format, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].copyFrom(other.bufs[i]);
    }
}

template<typename T>
void UnitBuf<T>::subtract(const UnitBuf<const T> &other) {
    CHECK_FATAL(chroma_format != other.chroma_format, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].subtract(other.bufs[i]);
    }
}

template<typename T>
void UnitBuf<T>::reconstruct(const UnitBuf<const T> &pred, const UnitBuf<const T> &resi, const ClpRng& clp_rngs) {
    CHECK_FATAL(chroma_format != pred.chroma_format, "Incompatible formats");
    CHECK_FATAL(chroma_format != resi.chroma_format, "Incompatible formats");

    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].reconstruct(pred.bufs[i], resi.bufs[i], clp_rngs);
    }
}

template<typename T>
void UnitBuf<T>::extend_border_pel(unsigned margin) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].extend_border_pel(margin);
    }
}

template<typename T>
void UnitBuf<T>::extend_border_pel(unsigned margin, bool left, bool right, bool top, bool bottom) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].extend_border_pel(margin, left, right, top, bottom);
    }
}

template<typename T>
void UnitBuf<T>::pad_border_pel(unsigned margin, int dir) {
    for(unsigned i = 0; i < bufs.size(); i++) {
        bufs[i].pad_border_pel(margin >> get_comp_scale_x(ComponentID(i), chroma_format), margin >> get_comp_scale_y(ComponentID(i), chroma_format), dir);
    }
}

template<typename T>
UnitBuf<T> UnitBuf<T>::sub_buf(const UnitArea& sub_area) {
  UnitBuf<T> sub_buf;
  sub_buf.chroma_format = chroma_format;
  unsigned block_idx = 0;

  for(auto &sub_area_buf : bufs) {
    sub_buf.bufs.push_back(sub_area_buf.sub_buf(sub_area.blocks[block_idx].pos(), sub_area.blocks[block_idx].size()));
    block_idx++;
  }

  return sub_buf;
}


template<typename T>
const UnitBuf<const T> UnitBuf<T>::sub_buf(const UnitArea& sub_area) const {
  UnitBuf<const T> sub_buf;
  sub_buf.chroma_format = chroma_format;
  unsigned block_idx = 0;

  for(const auto &sub_area_buf : bufs) {
    sub_buf.bufs.push_back(sub_area_buf.sub_buf(sub_area.blocks[block_idx].pos(), sub_area.blocks[block_idx].size()));
    block_idx++;
  }

  return sub_buf;
}

template<typename T>
UnitBuf<T> UnitBuf<T>::sub_buf(const Area & sub_area) {
  UnitBuf<T> sub_buf;
  sub_buf.chroma_format = chroma_format;
  unsigned block_idx = 0;

  for(auto &sub_area_buf : bufs) {
    const int scale_x = get_comp_scale_x(ComponentID(block_idx), chroma_format);
    const int scale_y = get_comp_scale_y(ComponentID(block_idx), chroma_format);
    const Area scaled_area(sub_area.pos().x >> scale_x, sub_area.pos().y >> scale_y, sub_area.size().width >> scale_x, sub_area.size().height >> scale_y);
    sub_buf.bufs.push_back(sub_area_buf.sub_buf(scaled_area.pos(), scaled_area.size()));
    block_idx++;
  }

  return sub_buf;
}

template<typename T>
const UnitBuf<const T> UnitBuf<T>::sub_buf(const Area & sub_area) const {
  UnitBuf<T> sub_buf;
  sub_buf.chroma_format = chroma_format;
  unsigned block_idx = 0;

  for(auto &sub_area_buf : bufs) {
    const int scale_x = get_comp_scale_x(ComponentID(block_idx), chroma_format);
    const int scale_y = get_comp_scale_y(ComponentID(block_idx), chroma_format);
    const Area scaled_area(sub_area.pos().x >> scale_x, sub_area.pos().y >> scale_y, sub_area.size().width >> scale_x, sub_area.size().height >> scale_y);
    sub_buf.bufs.push_back(sub_area_buf.sub_buf(scaled_area.pos(), scaled_area.size()));
    block_idx++;
  }

  return sub_buf;
}

PelStorage::PelStorage() {
    for(uint32_t i = 0; i < MAX_NUM_COMPONENT; i++)
    {
        m_origin[i]        = nullptr;
    }
}

PelStorage::~PelStorage() {
    destroy();
}

void PelStorage::swap(PelStorage& other) {
    const uint32_t num_ch = get_number_valid_comps(chroma_format);

    for(uint32_t i = 0; i < num_ch; i++) {
        // check this otherwise it would turn out to get very weird
        CHECK_FATAL(chroma_format              != other.chroma_format             , "Incompatible formats");
        CHECK_FATAL(get(ComponentID(i))        != other.get(ComponentID(i))       , "Incompatible formats");
        CHECK_FATAL(get(ComponentID(i)).stride != other.get(ComponentID(i)).stride, "Incompatible formats");

        std::swap(bufs[i].buf,    other.bufs[i].buf);
        std::swap(bufs[i].stride, other.bufs[i].stride);
        std::swap(m_origin[i],    other.m_origin[i]);
    }
}

void PelStorage::create_from_buf(PelUnitBuf buf) {
    chroma_format = buf.chroma_format;

    const uint32_t num_ch = get_number_valid_comps(chroma_format);

    bufs.resize(num_ch);

    for(uint32_t i = 0; i < num_ch; i++) {
        PelBuf cPelBuf = buf.get(ComponentID(i));
        bufs[i] = PelBuf(cPelBuf.buf_at(0, 0), cPelBuf.stride, cPelBuf.width, cPelBuf.height);
    }
}

void PelStorage::create(const UnitArea &_UnitArea) {
    create(_UnitArea.chroma_format, _UnitArea.blocks[0]);
}

void PelStorage::create(const ChromaFormat _chroma_format, const Size& _size, const unsigned _max_cu_size, const unsigned _margin, const unsigned _alignment_byte, const bool _scale_chroma_margin) {
    CHECK_FATAL(!bufs.empty(), "Trying to re-create an already initialized buffer");

    chroma_format = _chroma_format;

    const uint32_t num_ch = get_number_valid_comps(_chroma_format);

    unsigned ext_height = _size.height;
    unsigned ext_width  = _size.width;

    if(_max_cu_size) {
        ext_height = ((_size.height + _max_cu_size - 1) / _max_cu_size) * _max_cu_size;
        ext_width  = ((_size.width  + _max_cu_size - 1) / _max_cu_size) * _max_cu_size;
    }

    const unsigned _alignment = _alignment_byte / sizeof(Pel);

    for(uint32_t i = 0; i < num_ch; i++) {
        const ComponentID comp_id = ComponentID(i);
        const unsigned scale_x = get_comp_scale_x(comp_id, _chroma_format);
        const unsigned scale_y = get_comp_scale_y(comp_id, _chroma_format);

        unsigned scaled_height = ext_height >> scale_y ;
        unsigned scaled_width  = ext_width  >> scale_x;
        unsigned ymargin      = _margin >> (_scale_chroma_margin ? scale_y : 0);
        unsigned xmargin      = _margin >> (_scale_chroma_margin ? scale_x : 0);

        if(_alignment && xmargin) {
            xmargin = ((xmargin + _alignment - 1) / _alignment) * _alignment;
        }

        SizeType total_width   = scaled_width + 2 * xmargin;
        SizeType total_height  = scaled_height +2 * ymargin;

        if(_alignment) {
            // make sure buffer lines are align
            CHECK_FATAL(_alignment_byte != MEMORY_ALIGN_DEF_SIZE, "Unsupported alignment");
            total_width = ((total_width + _alignment - 1) / _alignment) * _alignment;
        }

        uint32_t area = total_width * total_height;
        CHECK_FATAL(!area, "Trying to create a buffer with zero area");

        m_orig_size[i] = Size{ total_width, total_height };
        m_origin[i] = (Pel*) x_malloc(Pel, area);
        Pel* topLeft = m_origin[i] + total_width * ymargin + xmargin;
        bufs.push_back(PelBuf(topLeft, total_width, _size.width >> scale_x, _size.height >> scale_y));
    }
}

void PelStorage::destroy() {
    chroma_format = NUM_CHROMA_FORMAT;
    for(uint32_t i = 0; i < MAX_NUM_COMPONENT; i++) {
        if(m_origin[i]) {
            m_origin[i] = nullptr;
        }
    }
    bufs.clear();
}

PelBuf PelStorage::get_buf(const ComponentID comp_id) {
    return bufs[comp_id];
}

const CPelBuf PelStorage::get_buf(const ComponentID comp_id) const {
    return bufs[comp_id];
}

PelBuf PelStorage::get_buf(const CompArea &blk) {
    const PelBuf& r = bufs[blk.comp_id()];

    CHECKD(rs_addr(blk.bottom_right(), r.stride) >= ((r.height - 1) * r.stride + r.width), "Trying to access a buf outside of bound!");

    return PelBuf(r.buf + rs_addr(blk, r.stride), r.stride, blk);
}

const CPelBuf PelStorage::get_buf(const CompArea &blk) const {
    const PelBuf& r = bufs[blk.comp_id()];
    return CPelBuf(r.buf + rs_addr(blk, r.stride), r.stride, blk);
}

PelUnitBuf PelStorage::get_buf(const UnitArea &unit) {
    return (chroma_format == CHROMA_400) ? PelUnitBuf(chroma_format, get_buf(unit.Y())) : PelUnitBuf(chroma_format, get_buf(unit.Y()), get_buf(unit.Cb()), get_buf(unit.Cr()));
}

const CPelUnitBuf PelStorage::get_buf(const UnitArea &unit) const {
    return (chroma_format == CHROMA_400) ? CPelUnitBuf(chroma_format, get_buf(unit.Y())) : CPelUnitBuf(chroma_format, get_buf(unit.Y()), get_buf(unit.Cb()), get_buf(unit.Cr()));
}