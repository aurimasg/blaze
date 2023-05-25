
#pragma once


#include <cstdint>
#include <cstring>
#include <cassert>
#include <cfloat>
#include <cmath>


#define FORCE_INLINE inline


#ifdef DEBUG
#define ASSERT(p) assert(p)
#else
#define ASSERT(p)
#endif


#if INTPTR_MAX == INT64_MAX
#define MACHINE_64
#undef MACHINE_32
#elif INTPTR_MAX == INT32_MAX
#define MACHINE_32
#undef MACHINE_64
#else
#error Weird pointer size!
#endif


#define SIZE_OF(p) sizeof(p)
#define BIT_SIZE_OF(p) (SIZE_OF(p) << 3)

#define STATIC_ASSERT(p) static_assert(p)

#define STATIC_ASSERT_UNSIGNED_TYPE(p) \
    STATIC_ASSERT(((p(1) << (BIT_SIZE_OF(p) - 1)) >> 1) == ((p(1) << (BIT_SIZE_OF(p) - 2))))


using int8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using uint64 = uint64_t;


STATIC_ASSERT(SIZE_OF(int8) == 1);
STATIC_ASSERT(SIZE_OF(uint8) == 1);
STATIC_ASSERT(SIZE_OF(int16) == 2);
STATIC_ASSERT(SIZE_OF(uint16) == 2);
STATIC_ASSERT(SIZE_OF(int32) == 4);
STATIC_ASSERT(SIZE_OF(uint32) == 4);
STATIC_ASSERT(SIZE_OF(int64) == 8);
STATIC_ASSERT(SIZE_OF(uint64) == 8);


#ifdef __GNUC__
#define ALIGNED(p) __attribute__((aligned((p))))
#else
#define ALIGNED(p)
#endif


#ifdef __GNUC__
#define LIKELY(a) __builtin_expect(!!(a), 1)
#define UNLIKELY(a) __builtin_expect(!!(a), 0)
#else
#define LIKELY(a) (a)
#define UNLIKELY(a) (a)
#endif


#define DISABLE_COPY_AND_ASSIGN(c) \
    c(const c &a) = delete; \
    void operator=(const c &a);


static FORCE_INLINE double Round(const double v) {
    return round(v);
}


static FORCE_INLINE float Round(const float v) {
    return roundf(v);
}


template <typename T>
static constexpr T Min(const T a, const T b) {
    return a < b ? a : b;
}


template <typename T>
static constexpr T Max(const T a, const T b) {
    return a > b ? a : b;
}


/**
 * Finds the smallest of the three values.
 */
template <typename T>
static constexpr T Min3(const T a, const T b, const T c) {
    return Min(a, Min(b, c));
}


/**
 * Finds the greatest of the three values.
 */
template <typename T>
static constexpr T Max3(const T a, const T b, const T c) {
    return Max(a, Max(b, c));
}


/**
 * Rounds-up a given number.
 */
static FORCE_INLINE float Ceil(const float v) {
    return ceilf(v);
}


/**
 * Rounds-up a given number.
 */
static FORCE_INLINE double Ceil(const double v) {
    return ceil(v);
}


/**
 * Rounds-down a given number.
 */
static FORCE_INLINE float Floor(const float v) {
    return floorf(v);
}


/**
 * Rounds-down a given number.
 */
static FORCE_INLINE double Floor(const double v) {
    return floor(v);
}


/**
 * Returns square root of a given number.
 */
static FORCE_INLINE float Sqrt(const float v) {
    return sqrtf(v);
}


/**
 * Returns square root of a given number.
 */
static FORCE_INLINE double Sqrt(const double v) {
    return sqrt(v);
}


/**
 * Returns value clamped to range between minimum and maximum values.
 */
template <typename T>
static constexpr T Clamp(const T val, const T min, const T max) {
    return val > max ? max : val < min ? min : val;
}


/**
 * Returns absolute of a given value.
 */
template <typename T>
static constexpr T Abs(const T t) {
    return t >= 0 ? t : -t;
}


/**
 * Returns true if a given floating point value is not a number.
 */
static constexpr bool IsNaN(const float x) {
    return x != x;
}


/**
 * Returns true if a given double precision floating point value is not a
 * number.
 */
static constexpr bool IsNaN(const double x) {
    return x != x;
}


/**
 * Returns true if a given double precision floating point number is finite.
 */
static FORCE_INLINE bool DoubleIsFinite(const double x) {
    // 0 × finite → 0
    // 0 × infinity → NaN
    // 0 × NaN → NaN
    const double p = x * 0;

    return !IsNaN(p);
}


/**
 * Linearly interpolate between A and B.
 * If t is 0, returns A.
 * If t is 1, returns B.
 * If t is something else, returns value linearly interpolated between A and B.
 */
template <typename T, typename V>
static FORCE_INLINE T InterpolateLinear(const T A, const T B, const V t) {
    ASSERT(t >= 0);
    ASSERT(t <= 1);

    return A + ((B - A) * t);
}


/**
 * Returns true if two given numbers are considered equal.
 */
static FORCE_INLINE bool FuzzyIsEqual(const double a, const double b) {
    return (fabs(a - b) < DBL_EPSILON);
}


/**
 * Returns true if two given numbers are considered equal.
 */
static FORCE_INLINE bool FuzzyIsEqual(const float a, const float b) {
    return (fabsf(a - b) < FLT_EPSILON);
}


/**
 * Returns true if a number can be considered being equal to zero.
 */
static FORCE_INLINE bool FuzzyIsZero(const double d) {
    return fabs(d) < DBL_EPSILON;
}


/**
 * Returns true if two given numbers are not considered equal.
 */
static FORCE_INLINE bool FuzzyNotEqual(const double a, const double b) {
    return (fabs(a - b) >= DBL_EPSILON);
}


/**
 * Returns true if two given numbers are not considered equal.
 */
static FORCE_INLINE bool FuzzyNotEqual(const float a, const float b) {
    return (fabsf(a - b) >= FLT_EPSILON);
}


/**
 * Returns true if a number can be considered being equal to zero.
 */
static FORCE_INLINE bool FuzzyIsZero(const float f) {
    return fabsf(f) < FLT_EPSILON;
}


/**
 * Returns true if a number can not be considered being equal to zero.
 */
static FORCE_INLINE bool FuzzyNotZero(const double d) {
    return fabs(d) >= DBL_EPSILON;
}


/**
 * Returns true if a number can not be considered being equal to zero.
 */
static FORCE_INLINE bool FuzzyNotZero(const float f) {
    return fabsf(f) >= FLT_EPSILON;
}


/**
 * Finds the greatest of the four values.
 */
template <typename T>
static constexpr T Max4(const T a, const T b, const T c, const T d) {
    return Max(a, Max(b, Max(c, d)));
}


/**
 * Finds the smallest of the four values.
 */
template <typename T>
static constexpr T Min4(const T a, const T b, const T c, const T d) {
    return Min(a, Min(b, Min(c, d)));
}


/**
 * Convert degrees to radians.
 */
static constexpr double Deg2Rad(const double x) {
    // pi / 180
    return x * 0.01745329251994329576923690768489;
}


/**
 * Convert radians to degrees.
 */
static constexpr double Rad2Deg(const double x) {
    // 180 / pi.
    return x * 57.295779513082320876798154814105;
}


/**
 * Calculates sine of a given number.
 */
static FORCE_INLINE float Sin(const float v) {
    return sinf(v);
}


/**
 * Calculates sine of a given number.
 */
static FORCE_INLINE double Sin(const double v) {
    return sin(v);
}


/**
 * Calculate cosine of a given number.
 */
static FORCE_INLINE float Cos(const float v) {
    return cosf(v);
}


/**
 * Calculate cosine of a given number.
 */
static FORCE_INLINE double Cos(const double v) {
    return cos(v);
}


/**
 * Returns tangent of a given number.
 */
static FORCE_INLINE float Tan(const float v) {
    return tanf(v);
}


/**
 * Returns tangent of a given number.
 */
static FORCE_INLINE double Tan(const double v) {
    return tan(v);
}
