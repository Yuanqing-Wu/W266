#pragma once

#include <cstring>

#include "Def.h"
#include "Common.h"

#define INCX(ptr, stride) { ptr++; }
#define INCY(ptr, stride) { ptr += (stride); }
#define OFFSETX(ptr, stride, x) { ptr += (x); }
#define OFFSETY(ptr, stride, y) { ptr += (y) * (stride); }
#define OFFSET(ptr, stride, x, y) { ptr += (x) + (y) * (stride); }
#define GET_OFFSETX(ptr, stride, x) ((ptr) + (x))
#define GET_OFFSETY(ptr, stride, y) ((ptr) + (y) * (stride))
#define GET_OFFSET(ptr, stride, x, y) ((ptr) + (x) + (y) * (stride))

#define SIZE_AWARE_PER_EL_OP(OP, INC)                       \
if((width & 7) == 0) {                                      \
    for(int y = 0; y < height; y++) {                       \
        for(int x = 0; x < width; x += 8) {                 \
            OP(x + 0);                                      \
            OP(x + 1);                                      \
            OP(x + 2);                                      \
            OP(x + 3);                                      \
            OP(x + 4);                                      \
            OP(x + 5);                                      \
            OP(x + 6);                                      \
            OP(x + 7);                                      \
        }                                                   \
                                                            \
        INC;                                                \
    }                                                       \
} else if((width & 3) == 0) {                               \
    for(int y = 0; y < height; y++) {                       \
        for(int x = 0; x < width; x += 4) {                 \
            OP(x + 0);                                      \
            OP(x + 1);                                      \
            OP(x + 2);                                      \
            OP(x + 3);                                      \
        }                                                   \
                                                            \
        INC;                                                \
    }                                                       \
} else if((width & 1) == 0) {                               \
    for(int y = 0; y < height; y++) {                       \
        for(int x = 0; x < width; x += 2) {                 \
            OP(x + 0);                                      \
            OP(x + 1);                                      \
        }                                                   \
                                                            \
        INC;                                                \
    }                                                       \
} else {                                                    \
    for(int y = 0; y < height; y++) {                       \
        for(int x = 0; x < width; x++) {                    \
            OP(x);                                          \
        }                                                   \
                                                            \
        INC;                                                \
    }                                                       \
}

template<typename T>
struct AreaBuf : public Size {
    T*        buf;
    ptrdiff_t stride;

    AreaBuf()                                                                                   : Size(),                  buf(NULL), stride(0)        { }
    AreaBuf(T *_buf, const Size &size)                                                          : Size(size),            buf(_buf), stride(size.width) { }
    AreaBuf(T *_buf, const ptrdiff_t &_stride, const Size &size)                                : Size(size),            buf(_buf), stride(_stride)    { }
    AreaBuf(T *_buf, const SizeType &_width, const SizeType &_height)                           : Size(_width, _height), buf(_buf), stride(_width)     { }
    AreaBuf(T *_buf, const ptrdiff_t &_stride, const SizeType &_width, const SizeType &_height) : Size(_width, _height), buf(_buf), stride(_stride)    { }

    AreaBuf(const AreaBuf&)  = default;
    AreaBuf(    AreaBuf&&) = default;
    AreaBuf& operator=(const AreaBuf&)  = default;
    AreaBuf& operator=(    AreaBuf&&) = default;

    // conversion from AreaBuf<const T> to AreaBuf<T>
    template<bool T_IS_CONST = std::is_const<T>::value>
    AreaBuf(const AreaBuf<typename std::remove_const_t<T>>& other, std::enable_if_t<T_IS_CONST>* = 0) : Size(other), buf(other.buf), stride(other.stride) { }

    void fill  (const T &val);
    void memset  (const int val);

    void copyFrom  (const AreaBuf<const T> &other);

    void reconstruct  (const AreaBuf<const T> &pred, const AreaBuf<const T> &resi, const ClpRng& clp_rng);

    void subtract  (const AreaBuf<const T> &other);
    void extendBorderPel  (unsigned margin);
    void extendBorderPel  (unsigned margin, bool left, bool right, bool top, bool bottom);
    void padBorderPel  (unsigned margin_x, unsigned margin_y, int dir); // ?

          T& at(const int &x, const int &y)          { return buf[y * stride + x]; }
    const T& at(const int &x, const int &y) const    { return buf[y * stride + x]; }

          T& at(const Position &pos)                 { return buf[pos.y * stride + pos.x]; }
    const T& at(const Position &pos) const           { return buf[pos.y * stride + pos.x]; }


          T* bufAt(const int &x, const int &y)       { return GET_OFFSET(buf, stride,     x,     y); }
    const T* bufAt(const int &x, const int &y) const { return GET_OFFSET(buf, stride,     x,     y); }
          T* bufAt(const Position& pos)              { return GET_OFFSET(buf, stride, pos.x, pos.y); }
    const T* bufAt(const Position& pos) const        { return GET_OFFSET(buf, stride, pos.x, pos.y); }

    AreaBuf<      T> subBuf(const Area &area)                                                         { return subBuf(area.pos(), area.size()); }
    AreaBuf<const T> subBuf(const Area &area)                                                   const { return subBuf(area.pos(), area.size()); }
    AreaBuf<      T> subBuf(const Position &pos, const Size &size)                                    { return AreaBuf<      T>(bufAt(pos), stride, size); }
    AreaBuf<const T> subBuf(const Position &pos, const Size &size)                              const { return AreaBuf<const T>(bufAt(pos), stride, size); }
    AreaBuf<      T> subBuf(const int &x, const int &y, const unsigned &_w, const unsigned &_h)       { return AreaBuf<      T>(bufAt(x, y), stride, _w, _h); }
    AreaBuf<const T> subBuf(const int &x, const int &y, const unsigned &_w, const unsigned &_h) const { return AreaBuf<const T>(bufAt(x, y), stride, _w, _h); }
};

typedef AreaBuf<      Pel>  PelBuf;
typedef AreaBuf<const Pel> CPelBuf;

typedef AreaBuf<      TCoeff>  CoeffBuf;
typedef AreaBuf<const TCoeff> CCoeffBuf;

typedef AreaBuf<TCoeffSig>        CoeffSigBuf;
typedef AreaBuf<const TCoeffSig> CCoeffSigBuf;

struct UnitArea;
struct CompArea;

template<typename T>
struct UnitBuf {
    typedef static_vector<AreaBuf<T>,       MAX_NUM_COMPONENT> UnitBufBuffers;

    ChromaFormat chromaFormat;
    UnitBufBuffers bufs;

    UnitBuf() : chromaFormat(NUM_CHROMA_FORMAT) { }
    UnitBuf(const ChromaFormat &_chromaFormat, const UnitBufBuffers&  _bufs) : chromaFormat(_chromaFormat), bufs(_bufs) { }
    UnitBuf(const ChromaFormat &_chromaFormat,       UnitBufBuffers&& _bufs) : chromaFormat(_chromaFormat), bufs(std::forward<UnitBufBuffers>(_bufs)) { }
    UnitBuf(const ChromaFormat &_chromaFormat, const AreaBuf<T>  &blkY) : chromaFormat(_chromaFormat), bufs{ blkY } { }
    UnitBuf(const ChromaFormat &_chromaFormat,       AreaBuf<T> &&blkY) : chromaFormat(_chromaFormat), bufs{ std::forward<AreaBuf<T> >(blkY) } { }
    UnitBuf(const ChromaFormat &_chromaFormat, const AreaBuf<T>  &blkY, const AreaBuf<T>  &blkCb, const AreaBuf<T>  &blkCr) : chromaFormat(_chromaFormat), bufs{ blkY, blkCb, blkCr } { if(chromaFormat == CHROMA_400) bufs.resize(1); }
    UnitBuf(const ChromaFormat &_chromaFormat,       AreaBuf<T> &&blkY,       AreaBuf<T> &&blkCb,       AreaBuf<T> &&blkCr) : chromaFormat(_chromaFormat), bufs{ std::forward<AreaBuf<T> >(blkY), std::forward<AreaBuf<T> >(blkCb), std::forward<AreaBuf<T> >(blkCr) } { if(chromaFormat == CHROMA_400) bufs.resize(1); }

    UnitBuf(const UnitBuf& other)  = default;
    UnitBuf(     UnitBuf&& other) = default;
    UnitBuf& operator=(const UnitBuf& other)  = default;
    UnitBuf& operator=(     UnitBuf&& other) = default;

    // conversion from UnitBuf<const T> to UnitBuf<T>
    template<bool T_IS_COST = std::is_const<T>::value>
    UnitBuf(const UnitBuf<typename std::remove_const_t<T>>& other, std::enable_if_t<T_IS_COST>* = 0) : chromaFormat(other.chromaFormat), bufs(other.bufs) { }

          AreaBuf<T>& get(const ComponentID comp)        { return bufs[comp]; }
    const AreaBuf<T>& get(const ComponentID comp)  const { return bufs[comp]; }

          AreaBuf<T>& Y()        { return bufs[0]; }
    const AreaBuf<T>& Y()  const { return bufs[0]; }
          AreaBuf<T>& Cb()       { return bufs[1]; }
    const AreaBuf<T>& Cb() const { return bufs[1]; }
          AreaBuf<T>& Cr()       { return bufs[2]; }
    const AreaBuf<T>& Cr() const { return bufs[2]; }

    void fill  (const T &val);
    void copyFrom  (const UnitBuf<const T> &other);
    void reconstruct  (const UnitBuf<const T> &pred, const UnitBuf<const T> &resi, const ClpRng& clp_rngs);
    void subtract  (const UnitBuf<const T> &other);
    void extendBorderPel  (unsigned margin);
    void extendBorderPel  (unsigned margin, bool left, bool right, bool top, bool bottom);
    void padBorderPel  (unsigned margin, int dir);

          UnitBuf<      T> subBuf (const Area& subArea);
    const UnitBuf<const T> subBuf (const Area& subArea) const;
          UnitBuf<      T> subBuf (const UnitArea& subArea);
    const UnitBuf<const T> subBuf (const UnitArea& subArea) const;
};

typedef UnitBuf<      Pel>  PelUnitBuf;
typedef UnitBuf<const Pel> CPelUnitBuf;

typedef UnitBuf<      TCoeff>  CoeffUnitBuf;
typedef UnitBuf<const TCoeff> CCoeffUnitBuf;

struct PelStorage : public PelUnitBuf {
    PelStorage();
    ~PelStorage();

    void swap(PelStorage& other);
    void createFromBuf(PelUnitBuf buf);
    void create(const UnitArea &_unit);
    void create(const ChromaFormat _chromaFormat, const Size& _size, const unsigned _maxCUSize = 0, const unsigned _margin = 0, const unsigned _alignment = 0, const bool _scaleChromaMargin = true);
    void destroy();

           PelBuf getBuf(const CompArea &blk);
    const CPelBuf getBuf(const CompArea &blk) const;

           PelBuf getBuf(const ComponentID compId);
    const CPelBuf getBuf(const ComponentID compId) const;

           PelUnitBuf getBuf(const UnitArea &unit);
    const CPelUnitBuf getBuf(const UnitArea &unit) const;
    Pel *getOrigin(const int id) const { return m_Origin[id]; }
    PelBuf getOriginBuf(const int id) { return PelBuf(m_Origin[id], m_OrigSize[id]); }

    Size  getBufSize(const int id)      const { return  m_OrigSize[id]; }

    private:

    Size    m_OrigSize[MAX_NUM_COMPONENT];
    Pel    *m_Origin[MAX_NUM_COMPONENT];
};