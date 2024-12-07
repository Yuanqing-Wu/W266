#include "decode.h"

void access_unit_default(AccessUnit *access_unit ) {
    if(nullptr == access_unit) {
        return;
    }

    access_unit->payload = NULL; 
    access_unit->payload_size = 0;
    access_unit->payload_used_size = 0;
    access_unit->cts = 0;
    access_unit->dts = 0;
    access_unit->cts_valid = false;
    access_unit->dts_valid = false;
    access_unit->rap = false;
}

AccessUnit* access_unit_alloc() {
    AccessUnit* access_unit = (AccessUnit*)malloc(sizeof(AccessUnit));
    if(nullptr == access_unit) {
        return nullptr;
    }
    access_unit_default(access_unit);
    return access_unit;
}

void access_unit_alloc_payload(AccessUnit *access_unit, int payload_size) {
    access_unit->payload = (unsigned char*)malloc(sizeof(unsigned char) * payload_size);
    if(nullptr == access_unit->payload) {
        return;
    }
    access_unit->payload_size = payload_size;
}

Decoder* decoder_open() {

}