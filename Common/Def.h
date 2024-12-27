#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include <limits>

typedef       int16_t         Pel;               ///< pixel type
typedef       int             TCoeff;            ///< transform coefficient
typedef       int16_t         TCoeffSig;
typedef       int16_t         TMatrixCoeff;      ///< transform matrix coefficient
typedef       int16_t         TFilterCoeff;      ///< filter coefficient
typedef       int             Intermediate_Int;  ///< used as intermediate value in calculations

static const uint64_t MAX_UINT64 = std::numeric_limits<uint64_t>::max();
static const uint32_t MAX_UINT   = std::numeric_limits<uint32_t>::max();
static const int      MAX_INT    = std::numeric_limits<int>     ::max();
static const uint8_t  MAX_UCHAR  = std::numeric_limits<int8_t>  ::max();
static const uint8_t  MAX_SCHAR  = std::numeric_limits<uint8_t> ::max();

template<typename T>
struct ClpRngTemplate {
    T min() const { return 0; }
    T max() const { return ((1 << bd) - 1);}
    int bd;
};

enum TreeType : uint8_t {
    TREE_D = 0, //default tree status (for single-tree slice, TREE_D means joint tree; for dual-tree I slice, TREE_D means TREE_L for luma and TREE_C for chroma)
    TREE_L = 1, //separate tree only contains luma (may split)
    TREE_C = 2, //separate tree only contains chroma (not split), to avoid small chroma block
};

enum ModeType : uint8_t {
    MODE_TYPE_ALL = 0, //all modes can try
    MODE_TYPE_INTER = 1, //can try inter
    MODE_TYPE_INTRA = 2, //can try intra, ibc, palette
};

enum PredMode : uint8_t {
    MODE_INTER                 = 0,     ///< inter-prediction mode
    MODE_INTRA                 = 1,     ///< intra-prediction mode
    MODE_IBC                   = 2,     ///< ibc-prediction mode
    NUMBER_OF_PREDICTION_MODES
};

typedef ClpRngTemplate<Pel> ClpRng;

template <typename T> constexpr static inline T clip3  (const T min, const T max, const T a) { return std::min<T> (std::max<T> (min, a) , max); }  ///< general min/max clip
template <typename T> constexpr static inline T clipBD (const T x, const int bit_depth)             { return clip3(T(0), T((1 << bit_depth) - 1), x); }
template <typename T> constexpr static inline T clipPel(const T a, const ClpRng& clp_rng)           { return clipBD(a, clp_rng. bd); }  ///< clip reconstruction

static const int MAX_NUM_REF_PICS =                                16; ///< max. number of pictures used for reference
static const int MAX_NUM_REF =                                     16; ///< max. number of entries in picture reference list
static const int MAX_QP =                                          63;
static const int NOT_VALID =                                       -1;

static const int MAX_NUM_SUB_PICS =                               255;
static const int MAX_NUM_LONG_TERM_REF_PICS =                      33;

static const int MAX_TLAYER =                                       7; ///< Explicit temporal layer QP offset - max number of temporal layer

static const int MAX_LADF_INTERVALS       =                         5; /// max number of luma adaptive deblocking filter qp offset intervals

static const int MAX_CU_DEPTH =                                     7; ///< log2(CTUSize)
static const int MAX_CU_SIZE =                        1<<MAX_CU_DEPTH;
static const int MIN_CU_LOG2 =                                      2;
static const int MIN_PU_SIZE =                                      4;
static const int MIN_TU_SIZE =                                      4;
static const int MAX_LOG2_TU_SIZE_PLUS_ONE =                        7; ///< log2(MAX_TU_SIZE) + 1

typedef       uint16_t        SplitSeries;       ///< used to encoded the splits that caused a particular CU size

#define CLASS_COPY_MOVE_DEFAULT(Class)        \
    Class(const Class&)            = default; \
    Class(Class&&)                 = default; \
    Class& operator=(const Class&) = default; \
    Class& operator=(Class&&)      = default;

#define CLASS_COPY_MOVE_DELETE(Class)        \
    Class(const Class&)            = delete; \
    Class(Class&&)                 = delete; \
    Class& operator=(const Class&) = delete; \
    Class& operator=(Class&&)      = delete;

enum SliceType {
    B_SLICE               = 0,
    P_SLICE               = 1,
    I_SLICE               = 2,
    NUMBER_OF_SLICE_TYPES
};

enum ChromaFormat : uint8_t {
    CHROMA_400        = 0,
    CHROMA_420        = 1,
    CHROMA_422        = 2,
    CHROMA_444        = 3,
    NUM_CHROMA_FORMAT
};

enum ChannelType : uint8_t {
  CHANNEL_TYPE_LUMA    = 0,
  CHANNEL_TYPE_CHROMA  = 1,
  MAX_NUM_CHANNEL_TYPE
};

enum ComponentID : uint8_t {
    COMPONENT_Y         = 0,
    COMPONENT_Cb        = 1,
    COMPONENT_Cr        = 2,
    MAX_NUM_COMPONENT,
    JOINT_CbCr          = MAX_NUM_COMPONENT,
    MAX_NUM_TBLOCKS     = MAX_NUM_COMPONENT
};

enum RefPicList : uint8_t {
  REF_PIC_LIST_0               = 0,   ///< reference list 0
  REF_PIC_LIST_1               = 1,   ///< reference list 1
  NUM_REF_PIC_LIST_01          = 2,
  REF_PIC_LIST_X               = 100  ///< special mark
};

enum NalUnitType {
    NAL_UNIT_CODED_SLICE_TRAIL = 0,   // 0
    NAL_UNIT_CODED_SLICE_STSA,        // 1
    NAL_UNIT_CODED_SLICE_RADL,        // 2
    NAL_UNIT_CODED_SLICE_RASL,        // 3

    NAL_UNIT_RESERVED_VCL_4,
    NAL_UNIT_RESERVED_VCL_5,
    NAL_UNIT_RESERVED_VCL_6,

    NAL_UNIT_CODED_SLICE_IDR_W_RADL,  // 7
    NAL_UNIT_CODED_SLICE_IDR_N_LP,    // 8
    NAL_UNIT_CODED_SLICE_CRA,         // 9
    NAL_UNIT_CODED_SLICE_GDR,         // 10

    NAL_UNIT_RESERVED_IRAP_VCL_11,

    NAL_UNIT_OPI,                     // 12
    NAL_UNIT_DCI,                     // 13
    NAL_UNIT_VPS,                     // 14
    NAL_UNIT_SPS,                     // 15
    NAL_UNIT_PPS,                     // 16
    NAL_UNIT_PREFIX_APS,              // 17
    NAL_UNIT_SUFFIX_APS,              // 18
    NAL_UNIT_PH,                      // 19
    NAL_UNIT_ACCESS_UNIT_DELIMITER,   // 20
    NAL_UNIT_EOS,                     // 21
    NAL_UNIT_EOB,                     // 22
    NAL_UNIT_PREFIX_SEI,              // 23
    NAL_UNIT_SUFFIX_SEI,              // 24
    NAL_UNIT_FD,                      // 25

    NAL_UNIT_RESERVED_NVCL_26,
    NAL_UNIT_RESERVED_NVCL_27,

    NAL_UNIT_UNSPECIFIED_28,
    NAL_UNIT_UNSPECIFIED_29,
    NAL_UNIT_UNSPECIFIED_30,
    NAL_UNIT_UNSPECIFIED_31,
    NAL_UNIT_INVALID
};

static inline ChannelType toChannelType             (const ComponentID id)                         { return (id==COMPONENT_Y) ? CHANNEL_TYPE_LUMA : CHANNEL_TYPE_CHROMA; }
static inline bool        isLuma                    (const ComponentID id)                         { return (id==COMPONENT_Y);                                           }
static inline bool        isLuma                    (const ChannelType id)                         { return (id==CHANNEL_TYPE_LUMA);                                     }
static inline bool        isChroma                  (const ComponentID id)                         { return (id!=COMPONENT_Y);                                           }
static inline bool        isChroma                  (const ChannelType id)                         { return (id!=CHANNEL_TYPE_LUMA);                                     }
static inline uint32_t    getChannalTypeScaleX      (const ChannelType id, const ChromaFormat fmt) { return (isLuma(id) || (fmt==CHROMA_444)) ? 0 : 1;                  }
static inline uint32_t    getChannalTypeScaleY      (const ChannelType id, const ChromaFormat fmt) { return (isLuma(id) || (fmt!=CHROMA_420)) ? 0 : 1;                  }
static inline uint32_t    getCompScaleX             (const ComponentID id, const ChromaFormat fmt) { return getChannalTypeScaleX(toChannelType(id), fmt);          }
static inline uint32_t    getCompScaleY             (const ComponentID id, const ChromaFormat fmt) { return getChannalTypeScaleY(toChannelType(id), fmt);          }
static inline uint32_t    getNumberValidComps       (const ChromaFormat fmt)                       { return (fmt==CHROMA_400) ? 1 : MAX_NUM_COMPONENT;                   }
static inline uint32_t    getNumberValidChannels    (const ChromaFormat fmt)                       { return (fmt==CHROMA_400) ? 1 : MAX_NUM_CHANNEL_TYPE;                }
static inline bool        isChromaEnabled           (const ChromaFormat fmt)                       { return !(fmt==CHROMA_400);                                          }
static inline ComponentID getFirstCompOfChannel     (const ChannelType id)                         { return (isLuma(id) ? COMPONENT_Y : COMPONENT_Cb);                  }

enum ScalingList1dStartIdx {
    SCALING_LIST_1D_START_2x2    = 0,
    SCALING_LIST_1D_START_4x4    = 2,
    SCALING_LIST_1D_START_8x8    = 8,
    SCALING_LIST_1D_START_16x16  = 14,
    SCALING_LIST_1D_START_32x32  = 20,
    SCALING_LIST_1D_START_64x64  = 26,
};

class Exception : public std::exception {
public:
    explicit Exception(const std::string& _s) : m_str(_s) {}
    virtual ~Exception() noexcept = default;
    CLASS_COPY_MOVE_DEFAULT(Exception)

    virtual const char* what() const noexcept  { return m_str.c_str(); }
    template<typename T>
    inline Exception& operator<<(const T& t) { std::ostringstream oss; oss << t; m_str += oss.str(); return *this; }

private:
    std::string m_str;
};

class RecoverableException : public Exception {
public:
    explicit RecoverableException( const std::string& _s ) : Exception( _s ) {}
    virtual ~RecoverableException() noexcept = default;
    CLASS_COPY_MOVE_DEFAULT( RecoverableException )

    template<typename T>
    inline RecoverableException& operator<<( const T& t ) { static_cast<Exception&>( *this ) << t; return *this; }
};

#define FMT_ERROR_LOCATION "In function \"" << __PRETTY_FUNCTION__ << "\" in " << __FILE__ ":" << __LINE__ << ": "

#define WARN(msg)                    { std::cerr << "\nWARNING: " << FMT_ERROR_LOCATION << msg << std::endl;          }
#define ABORT(msg)                   { std::cerr << "\nERROR: "   << FMT_ERROR_LOCATION << msg << std::endl; abort(); }

#define THROW_FATAL(msg)             throw(Exception                  ("\nERROR: ") << FMT_ERROR_LOCATION << msg)
#define THROW_RECOVERABLE( msg )     throw( RecoverableException       ( "\nERROR: " ) << FMT_ERROR_LOCATION << msg )

#define LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define UNLIKELY(expr) (__builtin_expect(!!(expr), 0))

#define CHECK_WARN(cond, msg)        { if UNLIKELY(cond)   { WARN             (msg << "\nWARNING CONDITION: " << #cond); } }
#define CHECK_FATAL(cond, msg)       { if UNLIKELY(cond)   { THROW_FATAL      (msg << "\nERROR CONDITION: "   << #cond); } }
#define CHECK( cond, msg )           { if UNLIKELY( cond ) { THROW_RECOVERABLE( msg << "\nERROR CONDITION: "   << #cond ); } }
#define CHECKD(cond, msg)            { if UNLIKELY( cond ) { ABORT            ( msg << "\nERROR CONDITION: "   << #cond ); } }


#define MEMORY_ALIGN_DEF_SIZE       32  // for use with avx2 (256 bit)

#define xMalloc(type, len) detail::aligned_malloc<type>(len, MEMORY_ALIGN_DEF_SIZE)

namespace detail {
    template<typename T>
    static inline T* aligned_malloc(size_t len, size_t alignement) {
        T* p = NULL;
        if(posix_memalign((void**) &p, alignement, sizeof(T) * (len))) {
        THROW_FATAL("posix_memalign failed");
        }
        return p;
    }
}   // namespace detail

template<typename T, size_t N>
class static_vector {
    T _arr[ N ];
    size_t _size = 0;

public:

    typedef T         value_type;
    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef T&        reference;
    typedef T const&  const_reference;
    typedef T*        pointer;
    typedef T const*  const_pointer;
    typedef T*        iterator;
    typedef T const*  const_iterator;

    static const size_type max_num_elements = N;

    static_vector()                                        = default;
    static_vector(const static_vector<T, N>&)            = default;
    static_vector(static_vector<T, N>&&)                 = default;
    static_vector& operator=(const static_vector<T, N>&) = default;
    static_vector& operator=(static_vector<T, N>&&)      = default;

    static_vector(size_t N_) : _size(N_)                     { CHECKD(_size > N, "capacity exceeded"); }
    static_vector(size_t N_, const T& _val) : _size(0)       { resize(N_, _val); }
    template<typename It>
    static_vector(It _it1, It _it2) : _size(0)               { while(_it1 < _it2) _arr[ _size++ ] = *_it1++; }
    template<typename Iterable,
            typename IS_ITERABLE = decltype(std::cbegin(std::declval<Iterable>()), std::cend(std::declval<Iterable>()))>
    explicit static_vector(const Iterable& iterable) : _size(0) { for(auto& e: iterable) { push_back(e); } }

    static_vector(std::initializer_list<T> _il) : _size(0) {
        typename std::initializer_list<T>::iterator _src1 = _il.begin();
        typename std::initializer_list<T>::iterator _src2 = _il.end();

        while(_src1 < _src2) _arr[ _size++ ] = *_src1++;

        CHECKD(_size > N, "capacity exceeded");
    }
    static_vector& operator=(std::initializer_list<T> _il) {
        _size = 0;

        typename std::initializer_list<T>::iterator _src1 = _il.begin();
        typename std::initializer_list<T>::iterator _src2 = _il.end();

        while(_src1 < _src2) _arr[ _size++ ] = *_src1++;

        CHECKD(_size > N, "capacity exceeded");
        return *this;
    }

    void resize_noinit(size_t N_)               { CHECKD(N_ > N, "capacity exceeded"); _size = N_; }
    void resize(size_t N_)                      { CHECKD(N_ > N, "capacity exceeded"); while(_size < N_) _arr[ _size++ ] = T() ; _size = N_; }
    void resize(size_t N_, const T& _val)       { CHECKD(N_ > N, "capacity exceeded"); while(_size < N_) _arr[ _size++ ] = _val; _size = N_; }
    void reserve(size_t N_)                     { CHECKD(N_ > N, "capacity exceeded"); }
    void push_back(const T& _val)               { CHECKD(_size >= N, "capacity exceeded"); _arr[ _size++ ] = _val; }
    void push_back(T&& val)                     { CHECKD(_size >= N, "capacity exceeded"); _arr[ _size++ ] = std::forward<T>(val); }
    void pop_back()                               { CHECKD(_size == 0, "calling pop_back on an empty vector"); _size--; }
    void pop_front()                              { CHECKD(_size == 0, "calling pop_front on an empty vector"); _size--; for(int i = 0; i < _size; i++) _arr[i] = _arr[i + 1]; }
    void clear()                                  { _size = 0; }
    reference       at(size_t _i)               { CHECKD(_i >= _size, "Trying to access an out-of-bound-element"); return _arr[ _i ]; }
    const_reference at(size_t _i) const         { CHECKD(_i >= _size, "Trying to access an out-of-bound-element"); return _arr[ _i ]; }
    reference       operator[](size_t _i)       { CHECKD(_i >= _size, "Trying to access an out-of-bound-element"); return _arr[ _i ]; }
    const_reference operator[](size_t _i) const { CHECKD(_i >= _size, "Trying to access an out-of-bound-element"); return _arr[ _i ]; }
    reference       front()                       { CHECKD(_size == 0, "Trying to access the first element of an empty vector"); return _arr[ 0 ]; }
    const_reference front() const                 { CHECKD(_size == 0, "Trying to access the first element of an empty vector"); return _arr[ 0 ]; }
    reference       back()                        { CHECKD(_size == 0, "Trying to access the last element of an empty vector");  return _arr[ _size - 1 ]; }
    const_reference back() const                  { CHECKD(_size == 0, "Trying to access the last element of an empty vector");  return _arr[ _size - 1 ]; }
    pointer         data()                        { return _arr; }
    const_pointer   data() const                  { return _arr; }
    iterator        begin()                       { return _arr; }
    const_iterator  begin() const                 { return _arr; }
    const_iterator  cbegin() const                { return _arr; }
    iterator        end()                         { return _arr + _size; }
    const_iterator  end() const                   { return _arr + _size; };
    const_iterator  cend() const                  { return _arr + _size; };
    size_type       size() const                  { return _size; };
    size_type       byte_size() const             { return _size * sizeof(T); }
    bool            empty() const                 { return _size == 0; }

    size_type       capacity() const              { return N; }
    size_type       max_size() const              { return N; }
    size_type       byte_capacity() const         { return sizeof(_arr); }

    void            erase(const_iterator _pos)  { iterator it   = begin() + (_pos - 1 - begin());
                                                    iterator last = end() - 1;
                                                    while(++it != last) *it = *(it + 1);
                                                    _size--; }
};