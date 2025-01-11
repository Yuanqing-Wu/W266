#include "Unit.h"

Position CompArea::chromaPos(const ChromaFormat chromaFormat) const {
    if (isLuma(compId())) {
        uint32_t scaleX = getComponentScaleX(compId(), chromaFormat);
        uint32_t scaleY = getComponentScaleY(compId(), chromaFormat);

        return Position(x >> scaleX, y >> scaleY);
    } else {
        return *this;
    }
}

Size CompArea::lumaSize(const ChromaFormat chromaFormat) const {
    if(isChroma(compId())) {
        uint32_t scaleX = getComponentScaleX(compId(), chromaFormat);
        uint32_t scaleY = getComponentScaleY(compId(), chromaFormat);
        return Size( width << scaleX, height << scaleY );
    } else {
        return *this;
    }
}

Size CompArea::chromaSize(const ChromaFormat chromaFormat) const {
    if(isLuma(compId())) {
        uint32_t scaleX = getComponentScaleX(compId(), chromaFormat);
        uint32_t scaleY = getComponentScaleY(compId(), chromaFormat);
        return Size(width >> scaleX, height >> scaleY);
    } else {
        return *this;
    }
}

Position CompArea::lumaPos(const ChromaFormat chromaFormat) const {
    if(isChroma(compId())) {
        uint32_t scaleX = getComponentScaleX(compId(), chromaFormat);
        uint32_t scaleY = getComponentScaleY(compId(), chromaFormat);
        return Position(x << scaleX, y << scaleY);
    } else {
        return *this;
    }
}

Position CompArea::compPos( const ChromaFormat chromaFormat, const ComponentID compId ) const {
    return isLuma( compId ) ? lumaPos( chromaFormat ) : chromaPos( chromaFormat );
}

Position CompArea::chanPos( const ChromaFormat chromaFormat, const ChannelType chType ) const {
    return isLuma( chType ) ? lumaPos( chromaFormat ) : chromaPos( chromaFormat );
}

UnitArea::UnitArea(const ChromaFormat _chromaFormat, const CompArea &blk_y) : chromaFormat(_chromaFormat), blocks { blk_y } {}

UnitArea::UnitArea(const ChromaFormat _chromaFormat,       CompArea &&blk_y) : chromaFormat(_chromaFormat), blocks { std::forward<CompArea>(blk_y) } {}

UnitArea::UnitArea(const ChromaFormat _chromaFormat, const CompArea &blk_y, const CompArea &blkCb, const CompArea &blkCr)  : chromaFormat(_chromaFormat), blocks { blk_y, blkCb, blkCr } {}

UnitArea::UnitArea(const ChromaFormat _chromaFormat,       CompArea &&blk_y,      CompArea &&blkCb,      CompArea &&blkCr) : chromaFormat(_chromaFormat), blocks { std::forward<CompArea>(blk_y), std::forward<CompArea>(blkCb), std::forward<CompArea>(blkCr) } {}

bool UnitArea::contains(const UnitArea& other) const {
    bool any = false;

    if(blocks[0].valid() && other.blocks[0].valid()) {
        any = true;
        if(!blocks[0].contains(other.blocks[0])) return false;
    }

    if(blocks[1].valid() && other.blocks[1].valid()) {
        any = true;
        if(!blocks[1].contains(other.blocks[1])) return false;
    }

    if(blocks[2].valid() && other.blocks[2].valid()) {
        any = true;
        if(!blocks[2].contains(other.blocks[2])) return false;
    }

    return any;
}

bool UnitArea::contains(const UnitArea& other, const ChannelType chType) const {
    if(chType == CHANNEL_TYPE_LUMA && blocks[0].valid() && other.blocks[0].valid()) {
        if(!blocks[0].contains( other.blocks[0])) return false;
        return true;
    }

    if(chType == CHANNEL_TYPE_LUMA) return false;

    bool any = false;

    if(blocks[1].valid() && other.blocks[1].valid()) {
        any = true;
        if(!blocks[1].contains(other.blocks[1])) return false;
    }

    if(blocks[2].valid() && other.blocks[2].valid()) {
        any = true;
        if(!blocks[2].contains(other.blocks[2])) return false;
    }

    return any;
}

void UnitArea::repositionTo(const UnitArea& unitArea) {
    for(uint32_t i = 0; i < blocks.size(); i++) {
        blocks[i].repositionTo(unitArea.blocks[i]);
    }
}

