#pragma once

#include "Common/Def.h"
#include "Common/PicListManager.h"
#include "Common/BitStream.h"

class DecLib;

#define CHECK_READ( cond, msg, val )            CHECK( cond, msg << " (read:" << val << ")" )
#define CHECK_READ_RANGE( val, min, max, name ) CHECK( (val) < (min) || (val) > (max), name << " out of bounds (read:" << (val) << ")." )

#  define X_READ_FLAG( name )                              const bool     name = xReadFlag ()
#  define X_READ_FLAG_idx( name, idx )                     const bool     name = xReadFlag ()

#  define X_READ_UVLC_NO_RANGE(  name         )            const uint32_t name = xReadUvlc (        )
#  define X_READ_SVLC_NO_RANGE(  name         )            const int32_t  name = xReadSvlc (        )
#  define X_READ_CODE_NO_RANGE(  name, length )            const uint32_t name = xReadCode ( length )
#  define X_READ_SCODE_NO_RANGE( name, length )            const int32_t  name = xReadSCode( length )

#  define X_READ_UVLC_NO_RANGE_idx(  name, idx         )   const uint32_t name = xReadUvlc (        )
#  define X_READ_SVLC_NO_RANGE_idx(  name, idx         )   const int32_t  name = xReadSvlc (        )
#  define X_READ_CODE_NO_RANGE_idx(  name, idx, length )   const uint32_t name = xReadCode ( length )
#  define X_READ_SCODE_NO_RANGE_idx( name, idx, length )   const int32_t  name = xReadSCode( length )

#define X_READ_FLAG_CHECK( name,          chk_cond, chk_msg ) const bool name = [&]{ X_READ_FLAG     ( name      ); CHECK_READ( chk_cond, chk_msg, name ); return name; }()
#define X_READ_FLAG_CHECK_idx( name, idx, chk_cond, chk_msg ) const bool name = [&]{ X_READ_FLAG_idx ( name, idx ); CHECK_READ( chk_cond, chk_msg, name ); return name; }()

// use immediately invoked lambdas, to ensure we get a warning, when the result is unused
#define X_READ_UVLC( name,         min, max )                 const uint32_t name = [&]{ X_READ_UVLC_NO_RANGE( name         ); CHECK_READ_RANGE( name, min, max, #name ); return name; }()
#define X_READ_SVLC( name,         min, max )                 const int32_t  name = [&]{ X_READ_SVLC_NO_RANGE( name         ); CHECK_READ_RANGE( name, min, max, #name ); return name; }()
#define X_READ_CODE( name, length, min, max )                 const uint32_t name = [&]{ X_READ_CODE_NO_RANGE( name, length ); CHECK_READ_RANGE( name, min, max, #name ); return name; }()
#define X_READ_SCODE(name, length, min, max )                 const int32_t  name = [&]{ X_READ_SCODE_NO_RANGE(name, length ); CHECK_READ_RANGE( name, min, max, #name ); return name; }()

#define X_READ_UVLC_idx( name, idx,         min, max )        const uint32_t name = [&]{ X_READ_UVLC_NO_RANGE_idx( name, idx         ); CHECK_READ_RANGE( name, min, max, (#name idx) ); return name; }()
#define X_READ_SVLC_idx( name, idx,         min, max )        const int32_t  name = [&]{ X_READ_SVLC_NO_RANGE_idx( name, idx         ); CHECK_READ_RANGE( name, min, max, (#name idx) ); return name; }()
#define X_READ_CODE_idx( name, idx, length, min, max )        const uint32_t name = [&]{ X_READ_CODE_NO_RANGE_idx( name, idx, length ); CHECK_READ_RANGE( name, min, max, (#name idx) ); return name; }()
#define X_READ_SCODE_idx(name, idx, length, min, max )        const int32_t  name = [&]{ X_READ_SCODE_NO_RANGE_idx(name, idx, length ); CHECK_READ_RANGE( name, min, max, (#name idx) ); return name; }()
class VLCReader {
protected:
    InputBitstream* m_pcBitstream = nullptr;   // we never own this object

    VLCReader()          = default;
    virtual ~VLCReader() = default;

    // read functions taking a reference for the result
    void xReadFlag (                  uint32_t& val );
    void xReadUvlc (                  uint32_t& val );
    void xReadSvlc (                  int32_t&  val );
    void xReadCode ( uint32_t length, uint32_t& val );
    void xReadSCode( uint32_t length, int32_t&  val );

    // read functions taking a reference for the result - tracing overloads
    void xReadFlag (                  uint32_t& rValue, const char* pSymbolName );
    void xReadUvlc (                  uint32_t& rValue, const char* pSymbolName );
    void xReadSvlc (                  int32_t&  rValue, const char* pSymbolName );
    void xReadCode ( uint32_t length, uint32_t& rValue, const char* pSymbolName );
    void xReadSCode( uint32_t length, int32_t&  rValue, const char* pSymbolName );

    // read functions returning the result value
    bool     xReadFlag();
    uint32_t xReadUvlc();
    int32_t  xReadSvlc();
    uint32_t xReadCode( uint32_t length );
    int32_t  xReadSCode( uint32_t length );

    // read functions returning the result value - tracing overloads
    bool     xReadFlag (                  const char* pSymbolName );
    uint32_t xReadUvlc (                  const char* pSymbolName );
    int32_t  xReadSvlc (                  const char* pSymbolName );
    uint32_t xReadCode ( uint32_t length, const char* pSymbolName );
    int32_t  xReadSCode( uint32_t length, const char* pSymbolName );

public:
    void            setBitstream( InputBitstream* p ) { m_pcBitstream = p; }
    InputBitstream* getBitstream() { return m_pcBitstream; }

protected:
    void xReadRbspTrailingBits();
    bool isByteAligned() { return ( m_pcBitstream->getNumBitsUntilByteAligned() == 0 ); }
};

class HLSyntaxReader : public VLCReader {
public:
    HLSyntaxReader()          = default;
    virtual ~HLSyntaxReader() = default;

    void  parseSPS                 ( SPS* pcSPS );
    void  parsePPS                 ( PPS* pcPPS );

    bool  xMoreRbspData();
};

struct NALUnit {
    NalUnitType m_nalUnitType = NAL_UNIT_INVALID;   ///< nal_unit_type
    uint32_t    m_temporalId  = 0;                  ///< temporal_id
    uint32_t    m_nuhLayerId  = 0;                  ///< nuh_layer_id
    uint32_t    m_forbiddenZeroBit   = 0;
    uint32_t    m_nuhReservedZeroBit = 0;

    uint64_t    m_bits     = 0;            ///< original nal unit bits
    uint64_t    m_cts      = 0;            ///< composition time stamp in TicksPerSecond
    uint64_t    m_dts      = 0;            ///< decoding time stamp in TicksPerSecond
    bool        m_rap      = false;        ///< random access point flag

    NALUnit(const NALUnit &src)
        : m_nalUnitType (src.m_nalUnitType)
        , m_temporalId  (src.m_temporalId)
        , m_nuhLayerId  (src.m_nuhLayerId)
        , m_forbiddenZeroBit  (src.m_forbiddenZeroBit)
        , m_nuhReservedZeroBit(src.m_nuhReservedZeroBit)
    {}

    /** construct an NALunit structure with given header values. */
    NALUnit( NalUnitType nalUnitType,
            int         temporalId = 0 ,
            int         nuhLayerId = 0 )
        : m_nalUnitType (nalUnitType)
        , m_temporalId  (temporalId)
        , m_nuhLayerId  (nuhLayerId)
    {}

    /** default constructor - no initialization; must be performed by user */
    NALUnit() {}

    virtual ~NALUnit() { }

    /** returns true if the NALunit is a slice NALunit */
    bool isSlice()
    {
        return m_nalUnitType == NAL_UNIT_CODED_SLICE_TRAIL
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_STSA
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_RADL
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_RASL
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_W_RADL
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_IDR_N_LP
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_CRA
            || m_nalUnitType == NAL_UNIT_CODED_SLICE_GDR
            ;
    }

    static bool isVclNalUnitType(NalUnitType t)
    {
        return t <= NAL_UNIT_RESERVED_IRAP_VCL_11;
    }  
};

class InputNALUnit : public NALUnit {
  private:
    InputBitstream m_Bitstream;

  public:
    InputNALUnit()  = default;
    ~InputNALUnit() = default;
    CLASS_COPY_MOVE_DEFAULT( InputNALUnit )

    const InputBitstream & getBitstream() const { return m_Bitstream; }
          InputBitstream & getBitstream()       { return m_Bitstream; }

    bool empty() { return m_Bitstream.getFifo().empty(); }

    void readNalUnitHeader();
};

class DecLibParser {
private:
    NalUnitType m_associatedIRAPType;   ///< NAL unit type of the associated IRAP picture
    int         m_pocCRA;   ///< POC number of the latest CRA picture
    int         m_pocRandomAccess      = MAX_INT;       ///< POC number of the random access point (the first IDR or CRA picture)
    int         m_decodingOrderCounter = 0;

    int m_prevPOC                   = MAX_INT;
    int m_prevTid0POC               = 0;

    DecLib&                   m_decLib;
    PicListManager&           m_picListManager;

    HLSyntaxReader            m_HLSReader;

public:
    DecLibParser( DecLib& decLib, PicListManager& picListManager ) : m_decLib( decLib ), m_picListManager( picListManager ) {}
    bool     parse                ( InputNALUnit& nalu );

    void xDecodeSPS             ( InputNALUnit& nalu );
};