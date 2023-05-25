
#pragma once


#include "Utils.h"


// BitVector is a fixed size bit array that fits into one register.
//
// Bit vector type should be either uint32 or uint64, depending on target CPU.
// It is tempting to just use an alias for something like uintptr_t, but it is
// easier for compiler-specific implementation to choose correct functions for
// builtins. See CountBits for GCC, for example. It has two implementations,
// for uint32 and uint64, calling either __builtin_popcount or
// __builtin_popcountl. And the rest of the API can use these functions
// without worrying that compiler will get confused which version to call.


#ifdef MACHINE_64
#include "BitOps_64.h"
#elif defined MACHINE_32
#include "BitOps_32.h"
#else
#error Unknown architecture
#endif


/**
 * Returns the number of bits set to 1 in a given value.
 *
 * @param v Value to count bits for. Must not be 0.
 */
template <typename T>
static constexpr int CountBits(const T v) {
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    ASSERT(v != 0);

    int num = 0;

    for (int i = 0; i < BIT_SIZE_OF(T); i++) {
        const T bit = v >> i;

        num += static_cast<int>(bit & 1);
    }

    return num;
}


/**
 * Returns the number of trailing zero bits in a given value, starting at the
 * least significant bit position.
 *
 * @param v Value to count trailing zeroes for. Must not be 0.
 */
template <typename T>
static constexpr int CountTrailingZeroes(const T v) {
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    ASSERT(v != 0);

    int i = 0;

    for ( ; i < BIT_SIZE_OF(T); i++) {
        const T bit = v >> i;
        const int m = static_cast<int>(bit & 1);

        if (m != 0) {
            return i;
        }
    }

    return i;
}


// Include compiler-specific bit ops.


#ifdef __GNUC__
#include "BitOps_gcc.h"
#endif


/**
 * Returns the amount of BitVector values needed to contain at least a given
 * amount of bits.
 *
 * @param maxBitCount Maximum number of bits for which storage is needed. Must
 * be at least 1.
 */
static constexpr int BitVectorsForMaxBitCount(const int maxBitCount) {
    ASSERT(maxBitCount);

    const int x = BIT_SIZE_OF(BitVector);

    return (maxBitCount + x - 1) / x;
}


/**
 * Calculates how many bits are set to 1 in bitmap.
 *
 * @param vec An array of BitVector values containing bits.
 *
 * @param count A number of values in vec. Note that this is not maximum
 * amount of bits to scan, but the amount of BitVector numbers vec contains.
 */
static constexpr int CountBitsInVector(const BitVector *vec, const int count) {
    int num = 0;

    for (int i = 0; i < count; i++) {
        const BitVector value = vec[i];

        if (value != 0) {
            num += CountBits(value);
        }
    }

    return num;
}


/**
 * Finds if bit at a given index is set to 1. If it is, this function returns
 * false. Otherwise, it sets bit at this index and returns true.
 *
 * @param vec Array of bit vectors. Must not be nullptr and must contain at
 * least (index + 1) amount of bits.
 *
 * @param index Bit index to test and set. Must be at least 0.
 */
template <typename T>
static constexpr bool ConditionalSetBit(T *vec, const int index) {
    STATIC_ASSERT_UNSIGNED_TYPE(T);

    ASSERT(vec != nullptr);
    ASSERT(index >= 0);

    const int vecIndex = index / BIT_SIZE_OF(T);

    T *v = vec + vecIndex;

    const int localIndex = index % BIT_SIZE_OF(T);
    const T current = *v;
    const T bit = T(1) << localIndex;

    if ((current & bit) == 0) {
        v[0] = current | bit;
        return true;
    }

    return false;
}


/**
 * Returns index to the first bit vector value which contains at least one bit
 * set to 1. If the entire array contains only zero bit vectors, an index to
 * the last bit vector will be returned.
 *
 * @param vec Bit vector array. Must not be nullptr.
 *
 * @param maxBitVectorCount A number of items in bit vector array. This
 * function always returns value less than this.
 */
static constexpr int FindFirstNonZeroBitVector(const BitVector *vec, const int maxBitVectorCount) {
    ASSERT(vec != nullptr);
    ASSERT(maxBitVectorCount > 0);

    int i = 0;

    for ( ; i < maxBitVectorCount; i++) {
        if (vec[i] != 0) {
            return i;
        }
    }

    return i;
}
