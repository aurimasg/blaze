
#pragma once


#include "FloatPoint.h"
#include "Utils.h"


/*
 * Roots must not be nullptr. Returns 0, 1 or 2.
 */
extern int FindQuadraticRoots(const double a, const double b, const double c,
    double roots[2]);


/**
 * Finds extrema on X axis of a quadratic curve and splits it at this point,
 * producing up to 2 output curves. Returns the number of output curves. If
 * extrema was not found, this function returns 1 and destination array
 * contains points of the original input curve. Always returns 1 or 2.
 *
 * Curves are stored in output as follows.
 *
 * 1. dst[0], dst[1], dst[2]
 * 2. dst[2], dst[3], dst[4]
 *
 * @param src Input quadratic curve as 3 points.
 *
 * @param dst Pointer to memory for destination curves. Must be large enough
 * to keep 5 FloatPoint values.
 */
extern int CutQuadraticAtXExtrema(const FloatPoint src[3], FloatPoint dst[5]);


/**
 * Finds extremas on X axis of a cubic curve and splits it at these points,
 * producing up to 3 output curves. Returns the number of output curves. If no
 * extremas are found, this function returns 1 and destination array contains
 * points of the original input curve. Always returns 1, 2 or 3.
 *
 * Curves are stored in output as follows.
 *
 * 1. dst[0], dst[1], dst[2], dst[3]
 * 2. dst[3], dst[4], dst[5], dst[6]
 * 3. dst[6], dst[7], dst[8], dst[9]
 *
 * @param src Input cubic curve as 4 points.
 *
 * @param dst Pointer to memory for destination curves. Must be large enough
 * to keep 10 FloatPoint values.
 */
extern int CutCubicAtXExtrema(const FloatPoint src[4], FloatPoint dst[10]);


/**
 * Finds extrema on Y axis of a quadratic curve and splits it at this point,
 * producing up to 2 output curves. Returns the number of output curves. If
 * extrema was not found, this function returns 1 and destination array
 * contains points of the original input curve. Always returns 1 or 2.
 *
 * Curves are stored in output as follows.
 *
 * 1. dst[0], dst[1], dst[2]
 * 2. dst[2], dst[3], dst[4]
 *
 * @param src Input quadratic curve as 3 points.
 *
 * @param dst Pointer to memory for destination curves. Must be large enough
 * to keep 5 FloatPoint values.
 */
extern int CutQuadraticAtYExtrema(const FloatPoint src[3], FloatPoint dst[5]);


/**
 * Finds extremas on Y axis of a cubic curve and splits it at these points,
 * producing up to 3 output curves. Returns the number of output curves. If no
 * extremas are found, this function returns 1 and destination array contains
 * points of the original input curve. Always returns 1, 2 or 3.
 *
 * Curves are stored in output as follows.
 *
 * 1. dst[0], dst[1], dst[2], dst[3]
 * 2. dst[3], dst[4], dst[5], dst[6]
 * 3. dst[6], dst[7], dst[8], dst[9]
 *
 * @param src Input cubic curve as 4 points.
 *
 * @param dst Pointer to memory for destination curves. Must be large enough
 * to keep 10 FloatPoint values.
 */
extern int CutCubicAtYExtrema(const FloatPoint src[4], FloatPoint dst[10]);



/**
 * Returns true if a given value is between a and b.
 */
static FORCE_INLINE bool IsValueBetweenAAndB(const double a, const double value, const double b) {
    if (a <= b) {
        return a <= value and value <= b;
    } else {
        return a >= value and value >= b;
    }
}


/**
 * Returns true if given cubic curve is monotonic in X. This function only
 * checks if cubic control points are between end points. This means that this
 * function can return false when in fact curve does not change direction in X.
 *
 * Use this function for fast monotonicity checks.
 */
static FORCE_INLINE bool CubicControlPointsBetweenEndPointsX(const FloatPoint pts[4]) {
    return
        IsValueBetweenAAndB(pts[0].X, pts[1].X, pts[3].X) and
        IsValueBetweenAAndB(pts[0].X, pts[2].X, pts[3].X);
}


static FORCE_INLINE bool QuadraticControlPointBetweenEndPointsX(const FloatPoint pts[3]) {
    return IsValueBetweenAAndB(pts[0].X, pts[1].X, pts[2].X);
}


/**
 * Returns true if given cubic curve is monotonic in Y. This function only
 * checks if cubic control points are between end points. This means that this
 * function can return false when in fact curve does not change direction in Y.
 *
 * Use this function for fast monotonicity checks.
 */
static FORCE_INLINE bool CubicControlPointsBetweenEndPointsY(const FloatPoint pts[4]) {
    return
        IsValueBetweenAAndB(pts[0].Y, pts[1].Y, pts[3].Y) and
        IsValueBetweenAAndB(pts[0].Y, pts[2].Y, pts[3].Y);
}


static FORCE_INLINE bool QuadraticControlPointBetweenEndPointsY(const FloatPoint pts[3]) {
    return IsValueBetweenAAndB(pts[0].Y, pts[1].Y, pts[2].Y);
}


static FORCE_INLINE void InterpolateQuadraticCoordinates(const double *src, double *dst, const double t) {
    ASSERT(t >= 0.0);
    ASSERT(t <= 1.0);

    const double ab = InterpolateLinear(src[0], src[2], t);
    const double bc = InterpolateLinear(src[2], src[4], t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = InterpolateLinear(ab, bc, t);
    dst[6] = bc;
    dst[8] = src[4];
}


static FORCE_INLINE void CutQuadraticAt(const FloatPoint src[3], FloatPoint dst[5], const double t) {
    ASSERT(t >= 0.0);
    ASSERT(t <= 1.0);

    InterpolateQuadraticCoordinates(&src[0].X, &dst[0].X, t);
    InterpolateQuadraticCoordinates(&src[0].Y, &dst[0].Y, t);
}


static FORCE_INLINE void InterpolateCubicCoordinates(const double *src, double *dst, const double t) {
    ASSERT(t >= 0.0);
    ASSERT(t <= 1.0);

    const double ab = InterpolateLinear(src[0], src[2], t);
    const double bc = InterpolateLinear(src[2], src[4], t);
    const double cd = InterpolateLinear(src[4], src[6], t);
    const double abc = InterpolateLinear(ab, bc, t);
    const double bcd = InterpolateLinear(bc, cd, t);
    const double abcd = InterpolateLinear(abc, bcd, t);

    dst[0] = src[0];
    dst[2] = ab;
    dst[4] = abc;
    dst[6] = abcd;
    dst[8] = bcd;
    dst[10] = cd;
    dst[12] = src[6];
}


static FORCE_INLINE void CutCubicAt(const FloatPoint src[4], FloatPoint dst[7], const double t) {
    ASSERT(t >= 0.0);
    ASSERT(t <= 1.0);

    InterpolateCubicCoordinates(&src[0].X, &dst[0].X, t);
    InterpolateCubicCoordinates(&src[0].Y, &dst[0].Y, t);
}
