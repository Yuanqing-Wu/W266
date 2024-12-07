#pragma once

#include "def.h"
#include "common.h"
#include "unit.h"

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

    void fill                 (const T &val);
    void memset               (const int val);

    void copy_from            (const AreaBuf<const T> &other);

    void reconstruct          (const AreaBuf<const T> &pred, const AreaBuf<const T> &resi, const ClpRng& clp_rng);

    void subtract             (const AreaBuf<const T> &other);
    void extend_border_pel    (unsigned margin);
    void extend_border_pel    (unsigned margin, bool left, bool right, bool top, bool bottom);
    void pad_border_pel       (unsigned margin_x, unsigned margin_y, int dir); // ?

          T& at(const int &x, const int &y)          { return buf[y * stride + x]; }
    const T& at(const int &x, const int &y) const    { return buf[y * stride + x]; }

          T& at(const Position &pos)                 { return buf[pos.y * stride + pos.x]; }
    const T& at(const Position &pos) const           { return buf[pos.y * stride + pos.x]; }


          T* buf_at(const int &x, const int &y)       { return GET_OFFSET(buf, stride,     x,     y); }
    const T* buf_at(const int &x, const int &y) const { return GET_OFFSET(buf, stride,     x,     y); }
          T* buf_at(const Position& pos)              { return GET_OFFSET(buf, stride, pos.x, pos.y); }
    const T* buf_at(const Position& pos) const        { return GET_OFFSET(buf, stride, pos.x, pos.y); }

    AreaBuf<      T> sub_buf(const Area &area)                                                         { return sub_buf(area.pos(), area.size()); }
    AreaBuf<const T> sub_buf(const Area &area)                                                   const { return sub_buf(area.pos(), area.size()); }
    AreaBuf<      T> sub_buf(const Position &pos, const Size &size)                                    { return AreaBuf<      T>(buf_at(pos), stride, size); }
    AreaBuf<const T> sub_buf(const Position &pos, const Size &size)                              const { return AreaBuf<const T>(buf_at(pos), stride, size); }
    AreaBuf<      T> sub_buf(const int &x, const int &y, const unsigned &_w, const unsigned &_h)       { return AreaBuf<      T>(buf_at(x, y), stride, _w, _h); }
    AreaBuf<const T> sub_buf(const int &x, const int &y, const unsigned &_w, const unsigned &_h) const { return AreaBuf<const T>(buf_at(x, y), stride, _w, _h); }
};

typedef AreaBuf<      Pel>  PelBuf;
typedef AreaBuf<const Pel> CPelBuf;

typedef AreaBuf<      TCoeff>  CoeffBuf;
typedef AreaBuf<const TCoeff> CCoeffBuf;

typedef AreaBuf<TCoeffSig>        CoeffSigBuf;
typedef AreaBuf<const TCoeffSig> CCoeffSigBuf;


template<typename T>
struct UnitBuf {
    typedef static_vector<AreaBuf<T>,       MAX_NUM_COMPONENT> UnitBufBuffers;

    ChromaFormat chroma_format;
    UnitBufBuffers bufs;

    UnitBuf() : chroma_format(NUM_CHROMA_FORMAT) { }
    UnitBuf(const ChromaFormat &_chroma_format, const UnitBufBuffers&  _bufs) : chroma_format(_chroma_format), bufs(_bufs) { }
    UnitBuf(const ChromaFormat &_chroma_format,       UnitBufBuffers&& _bufs) : chroma_format(_chroma_format), bufs(std::forward<UnitBufBuffers>(_bufs)) { }
    UnitBuf(const ChromaFormat &_chroma_format, const AreaBuf<T>  &blkY) : chroma_format(_chroma_format), bufs{ blkY } { }
    UnitBuf(const ChromaFormat &_chroma_format,       AreaBuf<T> &&blkY) : chroma_format(_chroma_format), bufs{ std::forward<AreaBuf<T> >(blkY) } { }
    UnitBuf(const ChromaFormat &_chroma_format, const AreaBuf<T>  &blkY, const AreaBuf<T>  &blkCb, const AreaBuf<T>  &blkCr) : chroma_format(_chroma_format), bufs{ blkY, blkCb, blkCr } { if(chroma_format == CHROMA_400) bufs.resize(1); }
    UnitBuf(const ChromaFormat &_chroma_format,       AreaBuf<T> &&blkY,       AreaBuf<T> &&blkCb,       AreaBuf<T> &&blkCr) : chroma_format(_chroma_format), bufs{ std::forward<AreaBuf<T> >(blkY), std::forward<AreaBuf<T> >(blkCb), std::forward<AreaBuf<T> >(blkCr) } { if(chroma_format == CHROMA_400) bufs.resize(1); }

    UnitBuf(const UnitBuf& other)  = default;
    UnitBuf(     UnitBuf&& other) = default;
    UnitBuf& operator=(const UnitBuf& other)  = default;
    UnitBuf& operator=(     UnitBuf&& other) = default;

    // conversion from UnitBuf<const T> to UnitBuf<T>
    template<bool T_IS_COST = std::is_const<T>::value>
    UnitBuf(const UnitBuf<typename std::remove_const_t<T>>& other, std::enable_if_t<T_IS_COST>* = 0) : chroma_format(other.chroma_format), bufs(other.bufs) { }

          AreaBuf<T>& get(const ComponentID comp)        { return bufs[comp]; }
    const AreaBuf<T>& get(const ComponentID comp)  const { return bufs[comp]; }

          AreaBuf<T>& Y()        { return bufs[0]; }
    const AreaBuf<T>& Y()  const { return bufs[0]; }
          AreaBuf<T>& Cb()       { return bufs[1]; }
    const AreaBuf<T>& Cb() const { return bufs[1]; }
          AreaBuf<T>& Cr()       { return bufs[2]; }
    const AreaBuf<T>& Cr() const { return bufs[2]; }

    void fill                 (const T &val);
    void copy_from            (const UnitBuf<const T> &other);
    void reconstruct          (const UnitBuf<const T> &pred, const UnitBuf<const T> &resi, const ClpRng& clp_rngs);
    void subtract             (const UnitBuf<const T> &other);
    void extend_border_pel    (unsigned margin);
    void extend_border_pel    (unsigned margin, bool left, bool right, bool top, bool bottom);
    void pad_border_pel       (unsigned margin, int dir);

          UnitBuf<      T> sub_buf (const Area& sub_area);
    const UnitBuf<const T> sub_buf (const Area& sub_area) const;
          UnitBuf<      T> sub_buf (const UnitArea& sub_area);
    const UnitBuf<const T> sub_buf (const UnitArea& sub_area) const;
};

typedef UnitBuf<      Pel>  PelUnitBuf;
typedef UnitBuf<const Pel> CPelUnitBuf;

typedef UnitBuf<      TCoeff>  CoeffUnitBuf;
typedef UnitBuf<const TCoeff> CCoeffUnitBuf;

struct PelStorage : public PelUnitBuf {
    PelStorage();
    ~PelStorage();

    void swap(PelStorage& other);
    void create_from_buf(PelUnitBuf buf);
    void create(const UnitArea &_unit);
    void create(const ChromaFormat _chromaFormat, const Size& _size, const unsigned _maxCUSize = 0, const unsigned _margin = 0, const unsigned _alignment = 0, const bool _scaleChromaMargin = true);
    void destroy();

           PelBuf get_buf(const CompArea &blk);
    const CPelBuf get_buf(const CompArea &blk) const;

           PelBuf get_buf(const ComponentID comp_id);
    const CPelBuf get_buf(const ComponentID comp_id) const;

           PelUnitBuf get_buf(const UnitArea &unit);
    const CPelUnitBuf get_buf(const UnitArea &unit) const;
    Pel *get_origin(const int id) const { return m_origin[id]; }
    PelBuf get_origin_buf(const int id) { return PelBuf(m_origin[id], m_orig_size[id]); }

    Size  get_buf_size(const int id)      const { return  m_orig_size[id]; }

    private:

    Size    m_orig_size[MAX_NUM_COMPONENT];
    Pel    *m_origin[MAX_NUM_COMPONENT];
};