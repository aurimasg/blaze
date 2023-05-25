
#pragma once


#include "Utils.h"


template <>
constexpr int CountBits<uint32>(const uint32 v) {
    ASSERT(v != 0);

    return __builtin_popcount(v);
}


template <>
constexpr int CountBits<uint64>(const uint64 v) {
    ASSERT(v != 0);

    return __builtin_popcountl(v);
}


template <>
constexpr int CountTrailingZeroes<uint32>(const uint32 v) {
    ASSERT(v != 0);

    return __builtin_ctz(v);
}


template <>
constexpr int CountTrailingZeroes<uint64>(const uint64 v) {
    ASSERT(v != 0);

    return __builtin_ctzl(v);
}
