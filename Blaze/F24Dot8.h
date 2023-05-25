
#pragma once


#include "Utils.h"


/**
 * 24.8 fixed point number.
 */
using F24Dot8 = int32;


STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);


/**
 * Value equivalent to one in 24.8 fixed point format.
 */
static constexpr F24Dot8 F24Dot8_1 = 1 << 8;


/**
 * Value equivalent to two in 24.8 fixed point format.
 */
static constexpr F24Dot8 F24Dot8_2 = 2 << 8;


/**
 * Converts double to 24.8 fixed point number. Does not check if a double is
 * small enough to be represented as 24.8 number.
 */
static FORCE_INLINE F24Dot8 DoubleToF24Dot8(const double v) {
    return F24Dot8(Round(v * 256.0));
}


/**
 * Returns absolute value for a given 24.8 fixed point number.
 */
static FORCE_INLINE F24Dot8 F24Dot8Abs(const F24Dot8 v) {
    STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);
    STATIC_ASSERT(SIZE_OF(int) == 4);

    const int mask = v >> 31;

    return (v + mask) ^ mask;
}
