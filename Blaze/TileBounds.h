
#pragma once


#include "Utils.h"


using TileIndex = uint32;


/**
 * Represents a rectangle in destination image coordinates, measured in tiles.
 */
struct TileBounds final {

    constexpr TileBounds(const TileIndex x, const TileIndex y,
        const TileIndex horizontalCount, const TileIndex verticalCount)
    :   X(x),
        Y(y),
        HorizontalCount(horizontalCount),
        VerticalCount(verticalCount)
    {
        ASSERT(HorizontalCount > 0);
        ASSERT(VerticalCount > 0);
    }

    // Minimum horizontal and vertical tile indices.
    TileIndex X = 0;
    TileIndex Y = 0;

    // Horizontal and vertical tile counts. Total number of tiles covered
    // by a geometry can be calculated by multiplying these two values.
    TileIndex HorizontalCount = 0;
    TileIndex VerticalCount = 0;
};
