
#pragma once


#include "Linearizer_p.h"
#include "TileBounds.h"
#include "TileDescriptor_8x8.h"
#include "TileDescriptor_8x16.h"
#include "TileDescriptor_8x32.h"
#include "TileDescriptor_16x8.h"
#include "TileDescriptor_64x16.h"


/**
 * Calculates column count for a given image width in pixels.
 *
 * @param width Image width in pixels. Must be at least 1.
 */
template <typename T>
static constexpr TileIndex CalculateColumnCount(const int width) {
    ASSERT(width > 0);

    return T::PointsToTileColumnIndex(width + T::TileW - 1);
}


/**
 * Calculates row count for a given image height in pixels.
 *
 * @param height Image height in pixels. Must be at least 1.
 */
template <typename T>
static constexpr TileIndex CalculateRowCount(const int height) {
    ASSERT(height > 0);

    return T::PointsToTileRowIndex(height + T::TileH - 1);
}


template <typename T>
static FORCE_INLINE TileBounds CalculateTileBounds(const int minx, const int miny, const int maxx, const int maxy) {
    ASSERT(minx >= 0);
    ASSERT(miny >= 0);
    ASSERT(minx < maxx);
    ASSERT(miny < maxy);

    const TileIndex x = T::PointsToTileColumnIndex(minx);
    const TileIndex y = T::PointsToTileRowIndex(miny);

    const TileIndex horizontalCount = T::PointsToTileColumnIndex(
        maxx + T::TileW - 1) - x;

    const TileIndex verticalCount = T::PointsToTileRowIndex(
        maxy + T::TileH - 1) - y;

    return TileBounds(x, y, horizontalCount, verticalCount);
}
