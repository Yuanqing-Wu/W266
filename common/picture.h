#pragma once

#include <limits>

#include "unit.h"
#include "buffer.h"

struct UnitArea;
struct PelStorage;
class CodingStructure;
class Slice;
struct Picture : public UnitArea {
    Picture() = default;
    ~Picture() = default;
    CLASS_COPY_MOVE_DELETE(Picture)


public:
    int         poc                 = 0;
    uint64_t    cts                 = 0;   // composition time stamp
    uint64_t    dts                 = 0;   // decoding time stamp
    uint32_t    tempLayer          = std::numeric_limits<uint32_t>::max();
    uint32_t    depth               = 0;
    int         layerId            = NOT_VALID;
    NalUnitType nalUnitType       = NAL_UNIT_INVALID;
    uint32_t    bits                = 0;   // input nal bit count
    bool        rap                 = 0;   // random access point flag
    int         decodingOrderNumber = 0;

    PelStorage     m_bufs;

    uint32_t       margin      = 0;

    CodingStructure*    cs = nullptr;
    std::vector<Slice*> slices;
};