
#pragma once


#include "Utils.h"
#include "F24Dot8.h"


/**
 * 8.8 fixed point number.
 */
using F8Dot8 = int16;

using F8Dot8x2 = uint32;
using F8Dot8x4 = uint64;


STATIC_ASSERT(SIZE_OF(F8Dot8) == 2);
STATIC_ASSERT(SIZE_OF(F8Dot8x2) == 4);
STATIC_ASSERT(SIZE_OF(F8Dot8x4) == 8);


static constexpr F8Dot8x2 PackF24Dot8ToF8Dot8x2(const F24Dot8 a, const F24Dot8 b) {
    STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);

    // Values must be small enough.
    ASSERT((a & 0xffff0000) == 0);
    ASSERT((b & 0xffff0000) == 0);

    return F8Dot8x2(a) | (F8Dot8x2(b) << 16);
}


static constexpr F8Dot8x4 PackF24Dot8ToF8Dot8x4(const F24Dot8 a, const F24Dot8 b, const F24Dot8 c, const F24Dot8 d) {
    STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);

    // Values must be small enough.
    ASSERT((a & 0xffff0000) == 0);
    ASSERT((b & 0xffff0000) == 0);
    ASSERT((c & 0xffff0000) == 0);
    ASSERT((d & 0xffff0000) == 0);

    return F8Dot8x4(a) | (F8Dot8x4(b) << 16) |
        (F8Dot8x4(c) << 32) | (F8Dot8x4(d) << 48);
}


static constexpr F24Dot8 UnpackLoFromF8Dot8x2(const F8Dot8x2 a) {
    return F24Dot8(a & 0xffff);
}


static constexpr F24Dot8 UnpackHiFromF8Dot8x2(const F8Dot8x2 a) {
    STATIC_ASSERT_UNSIGNED_TYPE(F8Dot8x2);

    return F24Dot8(a >> 16);
}
