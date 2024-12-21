#pragma once

#include <stdint.h>
#include <cmath>

#include "Def.h"

typedef int32_t PosType;
typedef uint32_t SizeType;

struct Position {
    PosType x;
    PosType y;

    constexpr Position()                                   : x(0),  y(0)    { }
    constexpr Position(const PosType _x, const PosType _y) : x(_x), y(_y)   { }

    constexpr bool operator!=(const Position &other)    const { return x != other.x || y != other.y; }
    constexpr bool operator==(const Position &other)    const { return x == other.x && y == other.y; }

    constexpr Position offset(const Position pos)                   const { return Position(x + pos.x, y + pos.y); }
    constexpr Position offset(const PosType _x, const PosType _y)   const { return Position(x + _x   , y + _y   ); }

    void repositionTo(const Position new_pos)   { x = new_pos.x; y = new_pos.y; }
    void relativeTo  (const Position origin)    { x -= origin.x; y -= origin.y; }

    constexpr Position operator-( const Position &other )   const { return{ x - other.x, y - other.y }; }
};

struct Size {
    SizeType width   : 32;
    SizeType height  : 30;
    uint8_t  _comp_id : 2;

    constexpr Size(const ComponentID c = MAX_NUM_COMPONENT)                                                 : width(0), height(0), _comp_id(c)   { }
    constexpr Size(const SizeType _width, const SizeType _height, const ComponentID c = MAX_NUM_COMPONENT ) : width(_width), height(_height), _comp_id(c)    { }

    constexpr bool operator!=(const Size &other)    const { return (width != other.width) || (height != other.height); }
    constexpr bool operator==(const Size &other)    const { return (width == other.width) && (height == other.height); }
    constexpr uint32_t area()                       const { return (uint32_t) width * (uint32_t) height; }

    const ComponentID compId()  const { return ComponentID( _comp_id ); }
};

struct Area : public Position, public Size {
    constexpr Area()                                                                                                                  : Position(), Size() { }
    constexpr Area(const Position &_pos, const Size &_size)                                                                           : Position(_pos), Size(_size) { }
    constexpr Area(const PosType _x, const PosType _y, const SizeType _w, const SizeType _h, const ComponentID c = MAX_NUM_COMPONENT) : Position(_x, _y), Size(_w, _h, c) { }

    Position&                 pos()                     { return *this; }
    constexpr const Position& pos()                     const { return *this; }

    Size& size() { return *this; }
    constexpr const Size& size()                        const { return *this; }

    constexpr const Position& topTeft()                 const { return *this; }
    constexpr Position topRight()                       const { return { (PosType) (x + width - 1), y }; }
    constexpr Position bottomLeft()                     const { return { x, (PosType) (y + height - 1) }; }
    constexpr Position bottomRight()                    const { return { (PosType) (x + width - 1), (PosType) (y + height - 1) }; }
    constexpr Position center()                         const { return { (PosType) (x + width / 2), (PosType) (y + height / 2) }; }

    constexpr bool contains(const Position &_pos)       const { return (_pos.x >= x) && (_pos.x < (x + width)) && (_pos.y >= y) && (_pos.y < (y + height)); }
    bool contains(const Area &_area)                    const { return contains(_area.pos()) && contains(_area.bottomRight()); }

    constexpr bool operator!=(const Area &other)        const { return (Size::operator!=(other)) || (Position::operator!=(other)); }
    constexpr bool operator==(const Area &other)        const { return (Size::operator==(other)) && (Position::operator==(other)); }
};

constexpr static inline ptrdiff_t rsAddr(const Position &pos, const ptrdiff_t stride) {
    return stride * (ptrdiff_t)pos.y + (ptrdiff_t)pos.x;
}

extern int8_t g_aucLog2[MAX_CU_SIZE + 1];
static inline int getLog2( int val ) {
    CHECKD( g_aucLog2[2] != 1, "g_aucLog2[] has not been initialized yet." );
    if( val > 0 && val < (int) sizeof( g_aucLog2 ) )
    {
        return g_aucLog2[val];
    }
    return std::log2( val );
}