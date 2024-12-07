#include "unit.h"

Position CompArea::chroma_pos(const ChromaFormat chroma_format) const {
    if (is_luma(comp_id())) {
        uint32_t scaleX = get_comp_scale_x(comp_id(), chroma_format);
        uint32_t scaleY = get_comp_scale_y(comp_id(), chroma_format);

        return Position(x >> scaleX, y >> scaleY);
    } else {
        return *this;
    }
}

Size CompArea::luma_size(const ChromaFormat chroma_format) const {
    if(is_chroma(comp_id())) {
        uint32_t scaleX = get_comp_scale_x(comp_id(), chroma_format);
        uint32_t scaleY = get_comp_scale_y(comp_id(), chroma_format);
        return Size( width << scaleX, height << scaleY );
    } else {
        return *this;
    }
}

Size CompArea::chroma_size(const ChromaFormat chroma_format) const {
    if(is_luma(comp_id())) {
        uint32_t scaleX = get_comp_scale_x(comp_id(), chroma_format);
        uint32_t scaleY = get_comp_scale_y(comp_id(), chroma_format);
        return Size(width >> scaleX, height >> scaleY);
    } else {
        return *this;
    }
}

Position CompArea::luma_pos(const ChromaFormat chroma_format) const {
    if(is_chroma(comp_id())) {
        uint32_t scaleX = get_comp_scale_x(comp_id(), chroma_format);
        uint32_t scaleY = get_comp_scale_y(comp_id(), chroma_format);
        return Position(x << scaleX, y << scaleY);
    } else {
        return *this;
    }
}

Position CompArea::comp_pos( const ChromaFormat chroma_format, const ComponentID comp_id ) const {
    return is_luma( comp_id ) ? luma_pos( chroma_format ) : chroma_pos( chroma_format );
}

Position CompArea::chan_pos( const ChromaFormat chroma_format, const ChannelType ch_type ) const {
    return is_luma( ch_type ) ? luma_pos( chroma_format ) : chroma_pos( chroma_format );
}

UnitArea::UnitArea(const ChromaFormat _chroma_format, const CompArea &blk_y) : chroma_format(_chroma_format), blocks { blk_y } {}

UnitArea::UnitArea(const ChromaFormat _chroma_format,       CompArea &&blk_y) : chroma_format(_chroma_format), blocks { std::forward<CompArea>(blk_y) } {}

UnitArea::UnitArea(const ChromaFormat _chroma_format, const CompArea &blk_y, const CompArea &blkCb, const CompArea &blkCr)  : chroma_format(_chroma_format), blocks { blk_y, blkCb, blkCr } {}

UnitArea::UnitArea(const ChromaFormat _chroma_format,       CompArea &&blk_y,      CompArea &&blkCb,      CompArea &&blkCr) : chroma_format(_chroma_format), blocks { std::forward<CompArea>(blk_y), std::forward<CompArea>(blkCb), std::forward<CompArea>(blkCr) } {}

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

bool UnitArea::contains(const UnitArea& other, const ChannelType ch_type) const {
    if(ch_type == CHANNEL_TYPE_LUMA && blocks[0].valid() && other.blocks[0].valid()) {
        if(!blocks[0].contains( other.blocks[0])) return false;
        return true;
    }

    if(ch_type == CHANNEL_TYPE_LUMA) return false;

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

void UnitArea::reposition_to(const UnitArea& unitArea) {
    for(uint32_t i = 0; i < blocks.size(); i++) {
        blocks[i].reposition_to(unitArea.blocks[i]);
    }
}

