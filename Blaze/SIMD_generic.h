
#include "Linearizer.h"
#include "SIMD.h"


static FORCE_INLINE F24Dot8 RoundTo24Dot8(const double v) {
    return static_cast<F24Dot8>(Round(v));
}


static FORCE_INLINE void FloatPointsToF24Dot8Points(const Matrix &matrix,
    F24Dot8Point *dst, const FloatPoint *src, const int count,
    const F24Dot8Point origin, const F24Dot8Point size)
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
            // Scale only matrix.
            const double sx = matrix.M11() * 256.0;
            const double sy = matrix.M22() * 256.0;

            for (int i = 0; i < count; i++) {
                dst[i].X = Clamp(RoundTo24Dot8(
                    src[i].X * sx) - origin.X, 0, size.X);

                dst[i].Y = Clamp(RoundTo24Dot8(
                    src[i].Y * sy) - origin.Y, 0, size.Y);
            }

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
