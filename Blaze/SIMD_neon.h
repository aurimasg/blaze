
#include "SIMD.h"
#include "Utils.h"

#include <arm_neon.h>


static FORCE_INLINE F24Dot8 RoundTo24Dot8(const double v) {
    return static_cast<F24Dot8>(Round(v));
}


static FORCE_INLINE void FloatToF24Dot8Points_ScaleOnly_neon(const Matrix &matrix,
    F24Dot8Point *dst, const FloatPoint *src, const int count,
    const F24Dot8Point origin, const F24Dot8Point size)
{
    const double sx = matrix.M11() * 256.0;
    const double sy = matrix.M22() * 256.0;

    // Combined scale and fixed point conversion multiplier.
    const float64x2_t s {
        sx,
        sy
    };

    const int32x4_t zero = vdupq_n_s32(0);

    // Origin translation vector.
    const int32x4_t translation {
        origin.X,
        origin.Y,
        origin.X,
        origin.Y
    };

    const int32x4_t max {
        size.X,
        size.Y,
        size.X,
        size.Y
    };

    // Do 4 points at a time.
    const int iterations = count >> 2;
    const double *ptr = reinterpret_cast<const double *>(src);
    int *dm = reinterpret_cast<int *>(dst);

    for (int i = 0; i < iterations; i++) {
        const float64x2_t v0 = vld1q_f64(ptr);
        const float64x2_t v1 = vld1q_f64(ptr + 2);
        const float64x2_t v2 = vld1q_f64(ptr + 4);
        const float64x2_t v3 = vld1q_f64(ptr + 6);

        ptr += 8;

        // Scale, multiply by 24.0 fixed point multiplier and round to
        // integers.
        const float64x2_t m0 = vrndq_f64(vmulq_f64(v0, s));
        const float64x2_t m1 = vrndq_f64(vmulq_f64(v1, s));
        const float64x2_t m2 = vrndq_f64(vmulq_f64(v2, s));
        const float64x2_t m3 = vrndq_f64(vmulq_f64(v3, s));

        const float32x2_t e0 = vcvt_f32_f64(m0);
        const float32x2_t e1 = vcvt_f32_f64(m1);
        const float32x4_t c0 = vcombine_f32(e0, e1);

        const float32x2_t e2 = vcvt_f32_f64(m2);
        const float32x2_t e3 = vcvt_f32_f64(m3);
        const float32x4_t c1 = vcombine_f32(e2, e3);

        // Apply origin translation, convert to integers and store.
        const int32x4_t i0 = vcvtq_s32_f32(c0);
        const int32x4_t i1 = vcvtq_s32_f32(c1);
        const int32x4_t t0 = vsubq_s32(i0, translation);
        const int32x4_t t1 = vsubq_s32(i1, translation);
        const int32x4_t k0 = vmaxq_s32(zero, vminq_s32(t0, max));
        const int32x4_t k1 = vmaxq_s32(zero, vminq_s32(t1, max));

        vst1q_s32(dm, k0);
        vst1q_s32(dm + 4, k1);

        dm += 8;
    }

    const int remainder = count & 3;

    for (int i = count - remainder; i < count; i++) {
        dst[i].X = Clamp(RoundTo24Dot8(
            src[i].X * sx) - origin.X, 0, size.X);

        dst[i].Y = Clamp(RoundTo24Dot8(
            src[i].Y * sy) - origin.Y, 0, size.Y);
    }
}


static FORCE_INLINE void FloatPointsToF24Dot8Points(const Matrix &matrix, F24Dot8Point *dst,
    const FloatPoint *src, const int count, const F24Dot8Point origin,
    const F24Dot8Point size)
{
    const MatrixComplexity complexity = matrix.DetermineComplexity();

    switch (complexity) {
        case MatrixComplexity::Identity: {
            // Identity matrix, convert only.
            for (int i = 0; i < count; i++) {
                dst[i].X = Clamp(DoubleToF24Dot8(
                    src[i].X) - origin.X, 0, size.X);

                dst[i].Y = Clamp(DoubleToF24Dot8(
                    src[i].Y) - origin.Y, 0, size.Y);
            }

            break;
        }

        case MatrixComplexity::TranslationOnly: {
            // Translation only matrix.
            const double tx = matrix.M31();
            const double ty = matrix.M32();

            for (int i = 0; i < count; i++) {
                dst[i].X = Clamp(DoubleToF24Dot8(
                    src[i].X + tx) - origin.X, 0, size.X);

                dst[i].Y = Clamp(DoubleToF24Dot8(
                    src[i].Y + ty) - origin.Y, 0, size.Y);
            }

            break;
        }

        case MatrixComplexity::ScaleOnly: {
            FloatToF24Dot8Points_ScaleOnly_neon(matrix, dst, src, count,
                origin, size);

            break;
        }

        case MatrixComplexity::TranslationScale: {
            // Scale and translation matrix.
            Matrix m(matrix);

            m.PreScale(256.0, 256.0);

            const double tx = m.M31();
            const double ty = m.M32();
            const double sx = m.M11();
            const double sy = m.M22();

            for (int i = 0; i < count; i++) {
                dst[i].X = Clamp(RoundTo24Dot8(
                    (src[i].X * sx) + tx) - origin.X, 0, size.X);

                dst[i].Y = Clamp(RoundTo24Dot8(
                    (src[i].Y * sy) + ty) - origin.Y, 0, size.Y);
            }

            break;
        }

        case MatrixComplexity::Complex: {
            Matrix m(matrix);

            m.PreScale(256.0, 256.0);

            const double m00 = m.M11();
            const double m01 = m.M12();
            const double m10 = m.M21();
            const double m11 = m.M22();
            const double m20 = m.M31();
            const double m21 = m.M32();

            for (int i = 0; i < count; i++) {
                const double x = src[i].X;
                const double y = src[i].Y;

                dst[i].X = Clamp(RoundTo24Dot8(
                    m00 * x + m10 * y + m20) - origin.X, 0, size.X);

                dst[i].Y = Clamp(RoundTo24Dot8(
                    m01 * x + m11 * y + m21) - origin.Y, 0, size.Y);
            }

            break;
        }
    }
}
