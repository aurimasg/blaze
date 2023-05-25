
#pragma once


#include "Utils.h"


/**
 * A simple struct which keeps a pointer to image data and associated
 * properties. It does not allocate or free any memory.
 */
struct ImageData final {

    /**
     * Construct image data.
     *
     * @param d Image data. It will be assigned, not copied. And it will not
     * be deallocated. This pointer must point to valid memory place as long
     * as image data struct is around.
     *
     * @param width Width in pixels. Must be at least 1.
     *
     * @param height Height in pixels. Must be at least 1.
     *
     * @param bytesPerRow Byte stride. Must be at least width × bpp.
     */
    constexpr ImageData(uint8 *d, const int width, const int height,
        const int bytesPerRow)
    :   Data(d),
        Width(width),
        Height(height),
        BytesPerRow(bytesPerRow)
    {
        ASSERT(width > 0);
        ASSERT(height > 0);

        // Do not assume any specific bpp, but assume it is at least 1 byte
        // per pixel.
        ASSERT(bytesPerRow >= width);
    }

    uint8 *Data = nullptr;
    const int Width = 0;
    const int Height = 0;
    const int BytesPerRow = 0;
};
