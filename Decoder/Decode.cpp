#include "Decode.h"
#include "Common/Rom.h"
#include "Common/Picture.h"

DecImpl::DecImpl() = default;
DecImpl::~DecImpl() = default;

int DecImpl::init() {
    m_cDecLib = std::make_unique<DecLib>();

    initROM();

    // create decoder class
    m_cDecLib->create();
    
    return 0;
}

NalType DecImpl::getNalUnitType (AccessUnit& rcAccessUnit) {
    NalType eNalType = VVC_NAL_UNIT_INVALID;

    if (rcAccessUnit.payload == nullptr ||
        rcAccessUnit.payloadSize < 3 ||
        rcAccessUnit.payloadUsedSize == 0) {
        return eNalType;
    }

    unsigned char* pcBuf = rcAccessUnit.payload;
    int iOffset=0;

    int found = 1;
    int i=0;
    for (i = 0; i < 3; i++) {
        if(pcBuf[i] != 0) {
            found = 0;
        }
    }

    if (pcBuf[i] != 1) {
        found = 0;
    }

    if (found) {
        iOffset=5;
    } else {
        found = 1;
        i=0;
        for (i = 0; i < 2; i++) {
            if(pcBuf[i] != 0) {
                found = 0;
            }
        }

        if (pcBuf[i] != 1) {
            found = 0;
        }

        if(found) {
            iOffset=4;
        }
    }

    if(found) {
        unsigned char uc = pcBuf[iOffset];
        int nalUnitType   = ((uc >> 3) & 0x1F );
        eNalType = (NalType)nalUnitType;
    }

    return eNalType;
}

int DecImpl::decode(AccessUnit& rcAccessUnit, Frame** ppcFrame) {
    InputNALUnit nalu;
    Picture * pcPic = nullptr;

    if( rcAccessUnit.payloadUsedSize ) {
        bool bStartCodeFound = false;
        std::vector<size_t> iStartCodePosVec;
        std::vector<size_t> iAUEndPosVec;
        std::vector<size_t> iStartCodeSizeVec;

        int pos = 0;
        while( pos+3 < rcAccessUnit.payloadUsedSize ) {
            int iFound = xRetrieveNalStartCode( &rcAccessUnit.payload[pos], 3 );
            if( iFound == 1 ) {
                bStartCodeFound = true;

                iStartCodePosVec.push_back( pos+4 );
                iStartCodeSizeVec.push_back( 4 );

                if( pos > 0 ) {
                    iAUEndPosVec.push_back(pos);
                }

                pos+=3;
            } else {
                iFound = xRetrieveNalStartCode(&rcAccessUnit.payload[pos], 2);
                if( iFound == 1 ) {
                    bStartCodeFound = true;

                    iStartCodePosVec.push_back( pos+3 );
                    iStartCodeSizeVec.push_back( 3 );
                    if( pos > 0 )
                    {
                        iAUEndPosVec.push_back(pos);
                    }

                    pos+=2;
                }
            }
            pos++;
        }

        // if( !bStartCodeFound ) {
        //     assert(0); //error
        // }

        int iLastPos = rcAccessUnit.payloadUsedSize;
        while( iLastPos > 0 && rcAccessUnit.payload[iLastPos-1] == 0 ) {
            iLastPos--;
        }
        iAUEndPosVec.push_back( iLastPos );

        InputBitstream& rBitstream = nalu.getBitstream();
        // iterate over all AU´s
        for( size_t iAU = 0; iAU < iStartCodePosVec.size(); iAU++ ) {
            //rBitstream.resetToStart();
            //rBitstream.getFifo().clear();
            //rBitstream.clearEmulationPreventionByteLocation();

            size_t numNaluBytes = iAUEndPosVec[iAU] - iStartCodePosVec[iAU];
            if( numNaluBytes ) {
                const uint8_t*    naluData = &rcAccessUnit.payload[iStartCodePosVec[iAU]];
                const NalUnitType nut      = (NalUnitType) ( ( naluData[1] >> 3 ) & 0x1f );
                // perform anti-emulation prevention
                // if( 0 != xConvertPayloadToRBSP( naluData, numNaluBytes, &rBitstream, NALUnit::isVclNalUnitType( nut ) ) )
                // {
                //     return VVDEC_ERR_UNSPECIFIED;
                // }

                // rBitstream.resetToStart();
                xConvertPayloadToRBSP( naluData, numNaluBytes, &rBitstream, NALUnit::isVclNalUnitType( nut ) );

                xReadNalUnitHeader( nalu );

                if( rcAccessUnit.ctsValid ){  nalu.m_cts = rcAccessUnit.cts; }
                if( rcAccessUnit.dtsValid ){  nalu.m_dts = rcAccessUnit.dts; }
                nalu.m_rap = rcAccessUnit.rap;
                nalu.m_bits = ( numNaluBytes + iStartCodeSizeVec[iAU] ) * 8;

                pcPic = m_cDecLib->decode( nalu );

                //iRet = xHandleOutput( pcPic );
                //assert(!iRet);
            }
        }
    }
    return 0;
}

int DecImpl::xRetrieveNalStartCode( unsigned char *pB, int iZerosInStartcode ) {
    int found = 1;
    int i=0;
    for ( i = 0; i < iZerosInStartcode; i++)
    {
        if( pB[i] != 0 )
        {
            found = 0;
        }
    }

    if( pB[i] != 1 )
    {
        found = 0;
    }

    return found;
}

int DecImpl::xReadNalUnitHeader(InputNALUnit& nalu)
{
    InputBitstream& bs = nalu.getBitstream();

    nalu.m_forbiddenZeroBit   = bs.read(1);                 // forbidden zero bit
    CHECK_WARN( nalu.m_forbiddenZeroBit != 0, "forbidden_zero_bit shall be equal to 0." );

    nalu.m_nuhReservedZeroBit = bs.read(1);                 // nuh_reserved_zero_bit
    CHECK_WARN( nalu.m_forbiddenZeroBit != 0, "nuh_reserved_zero_bit shall be equal to 0." );

    nalu.m_nuhLayerId         = bs.read(6);                 // nuh_layer_id
    if( nalu.m_nuhLayerId > 55 )
    {
        return -1;
    }

    nalu.m_nalUnitType        = (NalUnitType) bs.read(5);   // nal_unit_type
    nalu.m_temporalId         = bs.read(3) - 1;             // nuh_temporal_id_plus1
    CHECK( nalu.m_temporalId + 1 == 0, "The value of nuh_temporal_id_plus1 shall not be equal to 0." );
    CHECK( nalu.m_nalUnitType >= NAL_UNIT_CODED_SLICE_IDR_W_RADL && nalu.m_nalUnitType <= NAL_UNIT_RESERVED_IRAP_VCL_11 && nalu.m_temporalId != 0,
                        "When nal_unit_type is in the range of IDR_W_RADL to RSV_IRAP_11, inclusive, TemporalId shall be equal to 0." );

    // only check these rules for base layer
    CHECK( nalu.m_nuhLayerId == 0 && nalu.m_temporalId == 0 && nalu.m_nalUnitType == NAL_UNIT_CODED_SLICE_STSA,
                        "When NAL unit type is equal to STSA_NUT, TemporalId shall not be equal to 0\n" );

    return 0;
}

int DecImpl::xConvertPayloadToRBSP( const uint8_t* payload, size_t payloadLen, InputBitstream* bitstream, bool isVclNalUnit )
{
    uint32_t zeroCount = 0;

    AlignedByteVec& nalUnitBuf = bitstream->getFifo();
    nalUnitBuf.resize( payloadLen );

    const uint8_t*           it_read  = payload;
    AlignedByteVec::iterator it_write = nalUnitBuf.begin();
    for( size_t pos = 0; pos < payloadLen; it_read++, it_write++, pos++ )
    {
        if(zeroCount >= 2 && *it_read < 0x03 )
        {
            return -1;
        }
        if (zeroCount == 2 && *it_read == 0x03)
        {
            pos++;
            it_read++;
            zeroCount = 0;
            if( pos >= payloadLen )
            {
                break;
            }

            if( *it_read > 0x03 )
            {
                return -1;
            }
        }
        zeroCount = (*it_read == 0x00) ? zeroCount+1 : 0;
        *it_write = *it_read;
    }

    if( zeroCount != 0 && !isVclNalUnit )
    {
        return -1;
    }

    if (isVclNalUnit)
    {
        // Remove cabac_zero_word from payload if present
        int n = 0;

        while (it_write[-1] == 0x00)
        {
            it_write--;
            n++;
        }
    }

    nalUnitBuf.resize(it_write - nalUnitBuf.begin());

    return 0;
}

void accessUnitDefault(AccessUnit *accessUnit) {
    if(nullptr == accessUnit) {
        return;
    }

    accessUnit->payload = NULL; 
    accessUnit->payloadSize = 0;
    accessUnit->payloadUsedSize = 0;
    accessUnit->cts = 0;
    accessUnit->dts = 0;
    accessUnit->ctsValid = false;
    accessUnit->dtsValid = false;
    accessUnit->rap = false;
}

AccessUnit* accessUnitAlloc() {
    AccessUnit* accessUnit = (AccessUnit*)malloc(sizeof(AccessUnit));
    if(nullptr == accessUnit) {
        return nullptr;
    }
    accessUnitDefault(accessUnit);
    return accessUnit;
}

void accessUnitAllocPayload(AccessUnit *accessUnit, int payloadSize) {
    accessUnit->payload = (unsigned char*)malloc(sizeof(unsigned char) * payloadSize);
    if(nullptr == accessUnit->payload) {
        return;
    }
    accessUnit->payloadSize = payloadSize;
}

Decoder* decoderOpen() {
    DecImpl* decCtx = new DecImpl();
    if (!decCtx) {
        std::cerr << "W266 [error]: cannot allocate memory for W266 decoder" << std::endl;
        return nullptr;
    }
    int ret = decCtx->init();
    if (ret != 0) {
        delete decCtx;
        std::cerr << "W266 [error]: cannot init the W266 decoder" << std::endl;
        return nullptr;
    }
    return (Decoder*)decCtx;
}

NalType getNalUnitType(AccessUnit *accessUnit) {
    if(nullptr == accessUnit) {
        return VVC_NAL_UNIT_INVALID;
    }
    return DecImpl::getNalUnitType(*accessUnit);
}

int decode(Decoder *dec, AccessUnit* accessUnit, Frame** frame) {
    *frame = nullptr;
    auto d = (DecImpl*)dec;
    d->decode(*accessUnit, frame);
    return 0;
}

bool handleFrame() {
    return true;
}

int decoderClose() {
    return 0;
}

void accessUnitFree() {
    
}