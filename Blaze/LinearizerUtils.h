
#pragma once


/**
 * Split quadratic curve in half.
 *
 * @param r Resulting curves. First curve will be represented as elements at
 * indices 0, 1 and 2. Second curve will be represented as elements at indices
 * 2, 3 and 4.
 *
 * @param s Source curve defined by three points.
 */
static FORCE_INLINE void SplitQuadratic(F24Dot8Point r[5], const F24Dot8Point s[3]) {
    ASSERT(r != nullptr);
    ASSERT(s != nullptr);

    const F24Dot8 m0x = (s[0].X + s[1].X) >> 1;
    const F24Dot8 m0y = (s[0].Y + s[1].Y) >> 1;
    const F24Dot8 m1x = (s[1].X + s[2].X) >> 1;
    const F24Dot8 m1y = (s[1].Y + s[2].Y) >> 1;
    const F24Dot8 mx = (m0x + m1x) >> 1;
    const F24Dot8 my = (m0y + m1y) >> 1;

    r[0] = s[0];
    r[1].X = m0x;
    r[1].Y = m0y;
    r[2].X = mx;
    r[2].Y = my;
    r[3].X = m1x;
    r[3].Y = m1y;
    r[4] = s[2];
}


/**
 * Split cubic curve in half.
 *
 * @param r Resulting curves. First curve will be represented as elements at
 * indices 0, 1, 2 and 3. Second curve will be represented as elements at
 * indices 3, 4, 5 and 6.
 *
 * @param s Source curve defined by four points.
 */
static FORCE_INLINE void SplitCubic(F24Dot8Point r[7], const F24Dot8Point s[4]) {
    ASSERT(r != nullptr);
    ASSERT(s != nullptr);

    const F24Dot8 m0x = (s[0].X + s[1].X) >> 1;
    const F24Dot8 m0y = (s[0].Y + s[1].Y) >> 1;
    const F24Dot8 m1x = (s[1].X + s[2].X) >> 1;
    const F24Dot8 m1y = (s[1].Y + s[2].Y) >> 1;
    const F24Dot8 m2x = (s[2].X + s[3].X) >> 1;
    const F24Dot8 m2y = (s[2].Y + s[3].Y) >> 1;
    const F24Dot8 m3x = (m0x + m1x) >> 1;
    const F24Dot8 m3y = (m0y + m1y) >> 1;
    const F24Dot8 m4x = (m1x + m2x) >> 1;
    const F24Dot8 m4y = (m1y + m2y) >> 1;
    const F24Dot8 mx = (m3x + m4x) >> 1;
    const F24Dot8 my = (m3y + m4y) >> 1;

    r[0] = s[0];
    r[1].X = m0x;
    r[1].Y = m0y;
    r[2].X = m3x;
    r[2].Y = m3y;
    r[3].X = mx;
    r[3].Y = my;
    r[4].X = m4x;
    r[4].Y = m4y;
    r[5].X = m2x;
    r[5].Y = m2y;
    r[6] = s[3];
}


static FORCE_INLINE bool CutMonotonicQuadraticAt(const double c0, const double c1, const double c2, const double target, double &t) {
    const double A = c0 - c1 - c1 + c2;
    const double B = 2 * (c1 - c0);
    const double C = c0 - target;

    double roots[2];

    const int count = FindQuadraticRoots(A, B, C, roots);

    if (count > 0) {
        t = roots[0];
        return true;
    }

    return false;
}


static FORCE_INLINE bool CutMonotonicQuadraticAtX(const FloatPoint quadratic[3], const double x, double &t) {
    ASSERT(quadratic != nullptr);

    return CutMonotonicQuadraticAt(quadratic[0].X, quadratic[1].X,
        quadratic[2].X, x, t);
}


static FORCE_INLINE bool CutMonotonicQuadraticAtY(const FloatPoint quadratic[3], const double y, double &t) {
    ASSERT(quadratic != nullptr);

    return CutMonotonicQuadraticAt(quadratic[0].Y, quadratic[1].Y,
        quadratic[2].Y, y, t);
}


static FORCE_INLINE bool CutMonotonicCubicAt(double &t, const double pts[4]) {
    static constexpr double Tolerance = 1e-7;

    double negative = 0;
    double positive = 0;

    if (pts[0] < 0) {
        if (pts[3] < 0) {
            return false;
        }

        negative = 0;
        positive = 1.0;
    } else if (pts[0] > 0) {
        if (pts[3] > 0) {
            return false;
        }

        negative = 1.0;
        positive = 0;
    } else {
        t = 0;
        return true;
    }

    do {
        const double m = (positive + negative) / 2.0;
        const double y01 = InterpolateLinear(pts[0], pts[1], m);
        const double y12 = InterpolateLinear(pts[1], pts[2], m);
        const double y23 = InterpolateLinear(pts[2], pts[3], m);
        const double y012 = InterpolateLinear(y01, y12, m);
        const double y123 = InterpolateLinear(y12, y23, m);
        const double y0123 = InterpolateLinear(y012, y123, m);

        if (y0123 == 0.0) {
            t = m;
            return true;
        }

        if (y0123 < 0.0) {
            negative = m;
        } else {
            positive = m;
        }
    } while (Abs(positive - negative) > Tolerance);

    t = (negative + positive) / 2.0;

    return true;
}


static FORCE_INLINE bool CutMonotonicCubicAtY(const FloatPoint pts[4], const double y, double &t) {
    double c[4] = {
        pts[0].Y - y,
        pts[1].Y - y,
        pts[2].Y - y,
        pts[3].Y - y
    };

    return CutMonotonicCubicAt(t, c);
}


static FORCE_INLINE bool CutMonotonicCubicAtX(const FloatPoint pts[4], const double x, double &t) {
    double c[4] = {
        pts[0].X - x,
        pts[1].X - x,
        pts[2].X - x,
        pts[3].X - x
    };

    return CutMonotonicCubicAt(t, c);
}


/**
 * Returns true if a given quadratic curve is flat enough to be interpreted as
 * line for rasterizer.
 */
static FORCE_INLINE bool IsQuadraticFlatEnough(const F24Dot8Point q[3]) {
    ASSERT(q != nullptr);

    if (q[0].X == q[2].X and q[0].Y == q[2].Y) {
        return true;
    }

    // Find middle point between start and end point.
    const F24Dot8 mx = (q[0].X + q[2].X) >> 1;
    const F24Dot8 my = (q[0].Y + q[2].Y) >> 1;

    // Calculate cheap distance between middle point and control point.
    const F24Dot8 dx = F24Dot8Abs(mx - q[1].X);
    const F24Dot8 dy = F24Dot8Abs(my - q[1].Y);

    // Add both distances together and compare with allowed error.
    const F24Dot8 dc = dx + dy;

    // 32 in 24.8 fixed point format is equal to 0.125.
    return dc <= 32;
}


static FORCE_INLINE bool IsCubicFlatEnough(const F24Dot8Point c[4]) {
    ASSERT(c != nullptr);

    static constexpr F24Dot8 Tolerance = F24Dot8_1 >> 1;

    return
        F24Dot8Abs(2 * c[0].X - 3 * c[1].X + c[3].X) <= Tolerance and
        F24Dot8Abs(2 * c[0].Y - 3 * c[1].Y + c[3].Y) <= Tolerance and
        F24Dot8Abs(c[0].X - 3 * c[2].X + 2 * c[3].X) <= Tolerance and
        F24Dot8Abs(c[0].Y - 3 * c[2].Y + 2 * c[3].Y) <= Tolerance;
}
