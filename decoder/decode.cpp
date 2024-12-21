#include "decode.h"

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

void decoderOpen() {

}