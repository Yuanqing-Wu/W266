#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <memory>
#include <map>

#include "Common/Def.h"
#include "DecLib.h"

#define MAX_CODED_PICTURE_SIZE  800000

typedef struct Decoder Decoder;
typedef struct Picture Picture;

typedef enum {
  W266_OK                    = 0,
  W266_ERR_UNSPECIFIED       = -1,
  W266_ERR_INITIALIZE        = -2,
  W266_ERR_ALLOCATE          = -3,
  W266_ERR_DEC_INPUT         = -4,
  W266_NOT_ENOUGH_MEM        = -5,
  W266_ERR_PARAMETER         = -7,
  W266_ERR_NOT_SUPPORTED     = -10,
  W266_ERR_RESTART_REQUIRED  = -11,
  W266_ERR_CPU               = -30,
  W266_TRY_AGAIN             = -40,
  W266_EOF                   = -50,
} ErrorCodes;

typedef struct AccessUnit {
    unsigned char* payload;
    int payloadSize;
    int payloadUsedSize;
    uint64_t cts;
    uint64_t dts;
    bool ctsValid;
    bool dtsValid;
    bool rap;
} AccessUnit;

typedef struct Plane {
    unsigned char *ptr;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t bytesPerSample;
    void *allocator;
} Plane;

typedef enum {
    VVC_NAL_UNIT_CODED_SLICE_TRAIL = 0,  // 0
    VVC_NAL_UNIT_CODED_SLICE_STSA,       // 1
    VVC_NAL_UNIT_CODED_SLICE_RADL,       // 2
    VVC_NAL_UNIT_CODED_SLICE_RASL,       // 3

    VVC_NAL_UNIT_RESERVED_VCL_4,
    VVC_NAL_UNIT_RESERVED_VCL_5,
    VVC_NAL_UNIT_RESERVED_VCL_6,

    VVC_NAL_UNIT_CODED_SLICE_IDR_W_RADL, // 7
    VVC_NAL_UNIT_CODED_SLICE_IDR_N_LP,   // 8
    VVC_NAL_UNIT_CODED_SLICE_CRA,        // 9
    VVC_NAL_UNIT_CODED_SLICE_GDR,        // 10

    VVC_NAL_UNIT_RESERVED_IRAP_VCL_11,
    VVC_NAL_UNIT_RESERVED_IRAP_VCL_12,

    VVC_NAL_UNIT_DCI,                    // 13
    VVC_NAL_UNIT_VPS,                    // 14
    VVC_NAL_UNIT_SPS,                    // 15
    VVC_NAL_UNIT_PPS,                    // 16
    VVC_NAL_UNIT_PREFIX_APS,             // 17
    VVC_NAL_UNIT_SUFFIX_APS,             // 18
    VVC_NAL_UNIT_PH,                     // 19
    VVC_NAL_UNIT_ACCESS_UNIT_DELIMITER,  // 20
    VVC_NAL_UNIT_EOS,                    // 21
    VVC_NAL_UNIT_EOB,                    // 22
    VVC_NAL_UNIT_PREFIX_SEI,             // 23
    VVC_NAL_UNIT_SUFFIX_SEI,             // 24
    VVC_NAL_UNIT_FD,                     // 25

    VVC_NAL_UNIT_RESERVED_NVCL_26,
    VVC_NAL_UNIT_RESERVED_NVCL_27,

    VVC_NAL_UNIT_UNSPECIFIED_28,
    VVC_NAL_UNIT_UNSPECIFIED_29,
    VVC_NAL_UNIT_UNSPECIFIED_30,
    VVC_NAL_UNIT_UNSPECIFIED_31,
    VVC_NAL_UNIT_INVALID
} NalType;

typedef struct PicAttributes {
    NalType nalType;
    SliceType sliceType;
    bool isRefPic;
    uint32_t temporalLayer;
    int64_t poc;
    uint32_t bits;
} PicAttributes;

typedef struct Frame {
    Plane planes[MAX_NUM_COMPONENT];
    uint32_t numPlanes;
    uint32_t width;
    uint32_t height;
    uint32_t bitDepth;
    uint64_t cts;
    bool ctsValid;
    PicAttributes *picAttributes;
} Frame;

class DecImpl {
public:

    enum InternalState {
        INTERNAL_STATE_UNINITIALIZED = 0,
        INTERNAL_STATE_INITIALIZED = 1,
        INTERNAL_STATE_TUNING_IN = 2,
        INTERNAL_STATE_DECODING = 3,
        INTERNAL_STATE_FLUSHING = 4,
        INTERNAL_STATE_FINALIZED = 5,
        INTERNAL_STATE_RESTART_REQUIRED = 6,
        INTERNAL_STATE_NOT_SUPPORTED = 7
    };

public:

    DecImpl();
    ~DecImpl();

    class FrameStorage {
    public:
        int allocateStorage(size_t size) {
            if(size == 0) {
                return W266_ERR_ALLOCATE;
            }
            m_ptr.reset(new unsigned char[size]);
            m_size = size;
            return 0;
        }

        int freeStorage() {
            if(!m_ptr) {
                return W266_ERR_ALLOCATE;
            }
            m_ptr.reset();
            m_size = 0;
            return 0;
        }

        unsigned char * getStorage() {
            return m_ptr.get();
        }

        bool isAllocated() {return !!m_ptr;}

    private:
        std::unique_ptr<unsigned char[]> m_ptr = nullptr;   // pointer to plane buffer
        size_t m_size = 0;
    };

public:

    int init();
    // int uninit();
    // int reset();
    static NalType getNalUnitType       (AccessUnit& accessUnit);

private:
    typedef std::tuple<Frame, Picture*> FrameListEntry;
    typedef std::map<uint64_t, FrameStorage> FrameStorageMap;
    typedef FrameStorageMap::value_type      FrameStorageMapType;

    std::unique_ptr<DecLib>                  m_cDecLib;
};

AccessUnit* accessUnitAlloc();
void accessUnitAllocPayload(AccessUnit *accessUnit, int payloadSize);
Decoder* decoderOpen();
NalType getNalUnitType(AccessUnit *accessUnit);
int decode(Decoder *dec, AccessUnit* accessUnit, Frame** frame);