#pragma once

#include <vector>

#include "def.h"
#include "common.h"

class  CodingStructure;
class  Slice;
struct CodingUnit;
class SPS;
class PPS;

inline Position recalcPosition(const ChromaFormat _cf, const ComponentID srcCid, const ComponentID dstCid, const Position &pos) {
    if(toChannelType(srcCid) == toChannelType(dstCid)) {
        return pos;
    } else if (isLuma(srcCid) && isChroma(dstCid)) {
        return Position(pos.x >> getCompScaleX(dstCid, _cf), pos.y >> getCompScaleY(dstCid, _cf));
    } else {
        return Position(pos.x << getCompScaleX(srcCid, _cf), pos.y << getCompScaleY(srcCid, _cf));
    }
}

struct CompArea : public Area {
    CompArea() : Area()                                                                                                                                                            { }
    CompArea(const ComponentID _compId, const Area &_area)                                                          : Area(_area.x, _area.y, _area.width, _area.height, _compId) { }
    CompArea(const ComponentID _compId, const Position& _pos, const Size& _size)                                    : Area(_pos, Size( _size.width, _size.height, _compId))      { }
    CompArea(const ComponentID _compId, const uint32_t _x, const uint32_t _y, const uint32_t _w, const uint32_t _h) : Area(_x, _y, _w, _h, _compId)                              { }

    Position chromaPos(const ChromaFormat chromaFormat) const;
    Position lumaPos  (const ChromaFormat chromaFormat) const;

    Size     chromaSize(const ChromaFormat chromaFormat) const;
    Size     lumaSize  (const ChromaFormat chromaFormat) const;

    Position compPos(const ChromaFormat chromaFormat, const ComponentID compId) const;
    Position chanPos(const ChromaFormat chromaFormat, const ChannelType ch_type) const;

    Position topLeftComp    (const ChromaFormat chromaFormat, const ComponentID _compId) const { return recalcPosition(chromaFormat, compId(), _compId, *this);                                                     }
    Position topRightComp   (const ChromaFormat chromaFormat, const ComponentID _compId) const { return recalcPosition(chromaFormat, compId(), _compId, { (PosType) (x + width - 1), y                          }); }
    Position bottomLeftComp (const ChromaFormat chromaFormat, const ComponentID _compId) const { return recalcPosition(chromaFormat, compId(), _compId, { x                        , (PosType) (y + height - 1 )}); }
    Position bottomRightComp(const ChromaFormat chromaFormat, const ComponentID _compId) const { return recalcPosition(chromaFormat, compId(), _compId, { (PosType) (x + width - 1), (PosType) (y + height - 1 )}); }

    bool valid() const { return compId() < MAX_NUM_TBLOCKS && width != 0 && height != 0; }

    bool operator==(const CompArea &other) const {
        if (compId() != other.compId()) return false;
        return Position::operator==(other) && Size::operator==(other);
    }

    bool operator!=(const CompArea &other) const { return !(operator==(other)); }

    void     repositionTo      (const Position& newPos)        { Position::repositionTo(newPos); }
    void     positionRelativeTo(const CompArea& origCompArea) { Position::relativeTo(origCompArea); }
};

typedef static_vector<CompArea, MAX_NUM_TBLOCKS> UnitBlocksType;

struct UnitArea {
    ChromaFormat      chromaFormat;
    UnitBlocksType    blocks;

    UnitArea() : chromaFormat(NUM_CHROMA_FORMAT) { }
    UnitArea(const ChromaFormat _chromaFormat);
    UnitArea(const ChromaFormat _chromaFormat, const Area &area);
    UnitArea(const ChromaFormat _chromaFormat, const CompArea  &blkY);
    UnitArea(const ChromaFormat _chromaFormat,       CompArea &&blkY);
    UnitArea(const ChromaFormat _chromaFormat, const CompArea  &blkY, const CompArea  &blkCb, const CompArea  &blkCr);
    UnitArea(const ChromaFormat _chromaFormat,       CompArea &&blkY,       CompArea &&blkCb,       CompArea &&blkCr);

          CompArea& Y()                                  { return blocks[COMPONENT_Y];  }
    const CompArea& Y()                            const { return blocks[COMPONENT_Y];  }
          CompArea& Cb()                                 { return blocks[COMPONENT_Cb]; }
    const CompArea& Cb()                           const { return blocks[COMPONENT_Cb]; }
          CompArea& Cr()                                 { return blocks[COMPONENT_Cr]; }
    const CompArea& Cr()                           const { return blocks[COMPONENT_Cr]; }

          CompArea& block(const ComponentID comp)        { return blocks[comp]; }
    const CompArea& block(const ComponentID comp)  const { return blocks[comp]; }

    bool contains(const UnitArea& other) const;
    bool contains(const UnitArea& other, const ChannelType chType) const;

          CompArea& operator[](const int n)       { return blocks[n]; }
    const CompArea& operator[](const int n) const { return blocks[n]; }

    bool operator==(const UnitArea &other) const {
        if (chromaFormat != other.chromaFormat)   return false;
        if (blocks.size() != other.blocks.size())   return false;

        for (uint32_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] != other.blocks[i]) return false;
        }

        return true;
    }

    void repositionTo(const UnitArea& unit);

    bool operator!=(const UnitArea &other) const { return !(*this == other); }

    const Position& lumaPos () const { return Y(); }
    const Size&     lumaSize() const { return Y(); }
    const Area&     luma_area() const { return Y(); }

    const Position& chromaPos () const { return Cb(); }
    const Size&     chromaSize() const { return Cb(); }
    const Area&     chromaArea() const { return Cb(); }

    const UnitArea  singleComp(const ComponentID compID) const;
    const UnitArea  singleChan(const ChannelType chType) const;

    SizeType  lwidth()  const { return Y().width; }  /*! luma width  */
    SizeType  lheight() const { return Y().height; } /*! luma height */

    PosType   lx() const { return Y().x; }           /*! luma x-pos */
    PosType   ly() const { return Y().y; }           /*! luma y-pos */

    bool valid() const { return chromaFormat != NUM_CHROMA_FORMAT && blocks.size() > 0; }
};
struct TransformUnit : public UnitArea {
    CodingUnit     *cu;
    TransformUnit  *next;
    unsigned        idx;

    uint8_t         maxScanPosX  [MAX_NUM_TBLOCKS];
    uint8_t         maxScanPosY  [MAX_NUM_TBLOCKS];
    int8_t          chromaQp     [2];

    uint8_t         _chType    : 2;
    uint8_t         cbf        : 3;

    ChannelType     chType()                const { return ChannelType( _chType ); }
    void            setChType( ChannelType ch )   { _chType = ch; }
};
struct CodingUnit : public UnitArea {
    TransformUnit     firstTU;
    TransformUnit    *lastTU;

    struct CtuData   *ctuData;
    CodingStructure  *cs;
    const Slice      *slice;
    const PPS        *pps;
    const SPS        *sps;
            CodingUnit *next;
    const CodingUnit *above;
    const CodingUnit *left;
    ptrdiff_t         predBufOff;
    uint32_t          idx;

    int8_t            intraDir [MAX_NUM_CHANNEL_TYPE];

    SplitSeries       splitSeries;

    int8_t            chromaQpAdj;
    int8_t            qp;
    uint8_t           _mergeType      : 2;
    uint8_t           qtDepth         : 3;
    uint8_t           depth           : 4;
    uint8_t           _chType         : 1;

    bool              _rootCbf        : 1;
    uint8_t           _treeType       : 2;
    uint8_t           _modeType       : 2;

    uint8_t           _predMode       : 2;

    uint8_t           _multiRefIdx    : 2;
    bool              planeCbfY       : 1;
    bool              planeCbfU       : 1;
    bool              planeCbfV       : 1;
};