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
    return nullptr;
}

NalType getNalUnitType(AccessUnit *accessUnit) {
    if(nullptr == accessUnit) {
        return VVC_NAL_UNIT_INVALID;
    }
    return DecImpl::getNalUnitType(*accessUnit);
}

int decode(Decoder *dec, AccessUnit* accessUnit, Frame** frame) {

}