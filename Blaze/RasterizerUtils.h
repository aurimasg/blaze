
#pragma once


#include "Utils.h"


using FillRuleFn = int32 (*)(const int32);


/**
 * Given area, calculate alpha in range 0-255 using non-zero fill rule.
 *
 * This function implements `min(abs(area), 1.0)`.
 */
static constexpr int32 AreaToAlphaNonZero(const int32 area) {
    STATIC_ASSERT(SIZE_OF(int32) == 4);

    const int32 aa = area >> 9;

    // Find absolute area value.
    const int32 mask = aa >> 31;
    const int32 aaabs = (aa + mask) ^ mask;

    // Clamp absolute area value to be 255 or less.
    return Min(aaabs, 255);
}


/**
 * Given area, calculate alpha in range 0-255 using even-odd fill rule.
 *
 * This function implements `abs(area - 2.0 × round(0.5 × area))`.
 */
static constexpr int32 AreaToAlphaEvenOdd(const int32 area) {
    STATIC_ASSERT(SIZE_OF(int32) == 4);

    const int32 aa = area >> 9;

    // Find absolute area value.
    const int32 mask = aa >> 31;
    const int32 aaabs = (aa + mask) ^ mask;

    const int32 aac = aaabs & 511;

    if (aac > 256) {
        return 512 - aac;
    }

    return Min(aac, 255);
}


/**
 * This function returns 1 if value is greater than zero and it is divisible
 * by 256 (equal to one in 24.8 format) without a reminder.
 */
static constexpr int FindAdjustment(const F24Dot8 value) {
    STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);

    // Will be set to 0 is value is zero or less. Otherwise it will be 1.
    const int lte0 = ~((value - 1) >> 31) & 1;

    // Will be set to 1 if value is divisible by 256 without a reminder.
    // Otherwise it will be 0.
    const int db256 = (((value & 255) - 1) >> 31) & 1;

    // Return 1 if both bits (more than zero and disisible by 256) are set.
    return lte0 & db256;
}
