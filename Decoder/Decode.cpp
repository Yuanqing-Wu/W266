#include "Decode.h"
#include "Common/Rom.h"
#include "Common/Picture.h"

DecImpl::DecImpl() = default;
DecImpl::~DecImpl() = default;

int DecImpl::init() {
    initROM();
    
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
    return nullptr;
}