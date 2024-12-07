#pragma once

#include <vector>

#include "def.h"
#include "common.h"

inline Position recalc_position(const ChromaFormat _cf, const ComponentID src_cid, const ComponentID dst_cid, const Position &pos) {
    if(to_channel_type(src_cid) == to_channel_type(dst_cid)) {
        return pos;
    } else if (is_luma(src_cid) && is_chroma(dst_cid)) {
        return Position(pos.x >> get_comp_scale_x(dst_cid, _cf), pos.y >> get_comp_scale_y(dst_cid, _cf));
    } else {
        return Position(pos.x << get_comp_scale_x(src_cid, _cf), pos.y << get_comp_scale_y(src_cid, _cf));
    }
}

struct CompArea : public Area {
    CompArea() : Area()                                                                                                                                                            { }
    CompArea(const ComponentID _comp_id, const Area &_area)                                                          : Area(_area.x, _area.y, _area.width, _area.height, _comp_id) { }
    CompArea(const ComponentID _comp_id, const Position& _pos, const Size& _size)                                    : Area(_pos, Size( _size.width, _size.height, _comp_id))      { }
    CompArea(const ComponentID _comp_id, const uint32_t _x, const uint32_t _y, const uint32_t _w, const uint32_t _h) : Area(_x, _y, _w, _h, _comp_id)                              { }

    Position chroma_pos(const ChromaFormat chroma_format) const;
    Position luma_pos  (const ChromaFormat chroma_format) const;

    Size     chroma_size(const ChromaFormat chroma_format) const;
    Size     luma_size  (const ChromaFormat chroma_format) const;

    Position comp_pos(const ChromaFormat chroma_format, const ComponentID comp_id) const;
    Position chan_pos(const ChromaFormat chroma_format, const ChannelType ch_type) const;

    Position top_left_comp    (const ChromaFormat chroma_format, const ComponentID _comp_id) const { return recalc_position(chroma_format, comp_id(), _comp_id, *this);                                                     }
    Position top_right_comp   (const ChromaFormat chroma_format, const ComponentID _comp_id) const { return recalc_position(chroma_format, comp_id(), _comp_id, { (PosType) (x + width - 1), y                          }); }
    Position bottom_left_comp (const ChromaFormat chroma_format, const ComponentID _comp_id) const { return recalc_position(chroma_format, comp_id(), _comp_id, { x                        , (PosType) (y + height - 1 )}); }
    Position bottom_right_comp(const ChromaFormat chroma_format, const ComponentID _comp_id) const { return recalc_position(chroma_format, comp_id(), _comp_id, { (PosType) (x + width - 1), (PosType) (y + height - 1 )}); }

    bool valid() const { return comp_id() < MAX_NUM_TBLOCKS && width != 0 && height != 0; }

    bool operator==(const CompArea &other) const {
        if (comp_id() != other.comp_id()) return false;
        return Position::operator==(other) && Size::operator==(other);
    }

    bool operator!=(const CompArea &other) const { return !(operator==(other)); }

    void     reposition_to       (const Position& new_pos)        { Position::reposition_to(new_pos); }
    void     position_relative_to(const CompArea& orig_comp_area) { Position::relative_to(orig_comp_area); }
};

typedef static_vector<CompArea, MAX_NUM_TBLOCKS> UnitBlocksType;

struct UnitArea {
    ChromaFormat      chroma_format;
    UnitBlocksType    blocks;

    UnitArea() : chroma_format(NUM_CHROMA_FORMAT) { }
    UnitArea(const ChromaFormat _chroma_format);
    UnitArea(const ChromaFormat _chroma_format, const Area &area);
    UnitArea(const ChromaFormat _chroma_format, const CompArea  &blkY);
    UnitArea(const ChromaFormat _chroma_format,       CompArea &&blkY);
    UnitArea(const ChromaFormat _chroma_format, const CompArea  &blkY, const CompArea  &blkCb, const CompArea  &blkCr);
    UnitArea(const ChromaFormat _chroma_format,       CompArea &&blkY,       CompArea &&blkCb,       CompArea &&blkCr);

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
        if (chroma_format != other.chroma_format)   return false;
        if (blocks.size() != other.blocks.size())   return false;

        for (uint32_t i = 0; i < blocks.size(); i++) {
            if (blocks[i] != other.blocks[i]) return false;
        }

        return true;
    }

    void reposition_to(const UnitArea& unit);

    bool operator!=(const UnitArea &other) const { return !(*this == other); }

    const Position& luma_pos () const { return Y(); }
    const Size&     luma_size() const { return Y(); }
    const Area&     luma_area() const { return Y(); }

    const Position& chroma_pos () const { return Cb(); }
    const Size&     chroma_size() const { return Cb(); }
    const Area&     chroma_area() const { return Cb(); }

    const UnitArea  single_comp(const ComponentID compID) const;
    const UnitArea  single_chan(const ChannelType chType) const;

    SizeType  lwidth()  const { return Y().width; }  /*! luma width  */
    SizeType  lheight() const { return Y().height; } /*! luma height */

    PosType   lx() const { return Y().x; }           /*! luma x-pos */
    PosType   ly() const { return Y().y; }           /*! luma y-pos */

    bool valid() const { return chroma_format != NUM_CHROMA_FORMAT && blocks.size() > 0; }
};