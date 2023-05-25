
#pragma once


#include "F24Dot8Point.h"
#include "Utils.h"


/**
 * Keeps maximum point for clipping.
 */
struct ClipBounds final {

    constexpr ClipBounds(const int maxx, const int maxy)
    :   MaxX(maxx),
        MaxY(maxy),
        FMax(F24Dot8Point {
            maxx << 8,
            maxy << 8
        })
    {
        ASSERT(maxx > 0);
        ASSERT(maxy > 0);
    }


    const double MaxX = 0;
    const double MaxY = 0;
    const F24Dot8Point FMax = {
        0, 0
    };

private:
    // Prevent creating this with empty bounds as this is most likely not an
    // intentional situation.
    ClipBounds() = delete;
};
