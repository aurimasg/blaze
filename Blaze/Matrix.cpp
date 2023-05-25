
#include "Matrix.h"


const Matrix Matrix::Identity;


Matrix::Matrix(const Matrix &matrix1, const Matrix &matrix2)
{
    m[0][0] = matrix2.m[0][0] * matrix1.m[0][0] + matrix2.m[0][1] *
        matrix1.m[1][0];

    m[0][1] = matrix2.m[0][0] * matrix1.m[0][1] + matrix2.m[0][1] *
        matrix1.m[1][1];

    m[1][0] = matrix2.m[1][0] * matrix1.m[0][0] + matrix2.m[1][1] *
        matrix1.m[1][0];

    m[1][1] = matrix2.m[1][0] * matrix1.m[0][1] + matrix2.m[1][1] *
        matrix1.m[1][1];

    m[2][0] = matrix2.m[2][0] * matrix1.m[0][0] + matrix2.m[2][1] *
        matrix1.m[1][0] + matrix1.m[2][0];

    m[2][1] = matrix2.m[2][0] * matrix1.m[0][1] + matrix2.m[2][1] *
        matrix1.m[1][1] + matrix1.m[2][1];
}


Matrix::Matrix(const FloatPoint &translation)
{
    m[0][0] = 1;
    m[0][1] = 0;
    m[1][0] = 0;
    m[1][1] = 1;
    m[2][0] = translation.X;
    m[2][1] = translation.Y;
}


Matrix Matrix::CreateRotation(const double degrees)
{
    if (FuzzyIsZero(degrees)) {
        return Matrix::Identity;
    }

    double c = 0;
    double s = 0;

    if (degrees == 90.0 or degrees == -270.0) {
        s = 1;
    } else if (degrees == 180.0 or degrees == -180.0) {
        c = -1;
    } else if (degrees == -90.0 or degrees == 270.0) {
        s = -1;
    } else {
        // Arbitrary rotation.
        const double radians = Deg2Rad(degrees);

        c = Cos(radians);
        s = Sin(radians);
    }

    return Matrix(c, s, -s, c, 0, 0);
}


Matrix Matrix::Lerp(const Matrix &matrix1, const Matrix &matrix2,
    const double t)
{
    return Matrix(
        matrix1.m[0][0] + (matrix2.m[0][0] - matrix1.m[0][0]) * t,
        matrix1.m[0][1] + (matrix2.m[0][1] - matrix1.m[0][1]) * t,
        matrix1.m[1][0] + (matrix2.m[1][0] - matrix1.m[1][0]) * t,
        matrix1.m[1][1] + (matrix2.m[1][1] - matrix1.m[1][1]) * t,
        matrix1.m[2][0] + (matrix2.m[2][0] - matrix1.m[2][0]) * t,
        matrix1.m[2][1] + (matrix2.m[2][1] - matrix1.m[2][1]) * t);
}


bool Matrix::Invert(Matrix &result) const
{
    const double det = GetDeterminant();

    if (FuzzyIsZero(det)) {
        result = Matrix::Identity;
        return false;
    }

    result = Matrix(
         m[1][1] / det,
        -m[0][1] / det,
        -m[1][0] / det,
         m[0][0] / det,
        (m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det,
        (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det);

    return true;
}


Matrix Matrix::Inverse() const
{
    const double det = GetDeterminant();

    if (FuzzyIsZero(det)) {
        return Matrix::Identity;
    }

    return Matrix(
         m[1][1] / det,
        -m[0][1] / det,
        -m[1][0] / det,
         m[0][0] / det,
        (m[1][0] * m[2][1] - m[1][1] * m[2][0]) / det,
        (m[0][1] * m[2][0] - m[0][0] * m[2][1]) / det);
}


FloatRect Matrix::Map(const FloatRect &rect) const
{
    const FloatPoint topLeft = Map(rect.MinX, rect.MinY);
    const FloatPoint topRight = Map(rect.MaxX, rect.MinY);
    const FloatPoint bottomLeft = Map(rect.MinX, rect.MaxY);
    const FloatPoint bottomRight = Map(rect.MaxX, rect.MaxY);

    const double minX = Min(topLeft.X, Min(topRight.X, Min(bottomLeft.X,
        bottomRight.X)));
    const double maxX = Max(topLeft.X, Max(topRight.X, Max(bottomLeft.X,
        bottomRight.X)));
    const double minY = Min(topLeft.Y, Min(topRight.Y, Min(bottomLeft.Y,
        bottomRight.Y)));
    const double maxY = Max(topLeft.Y, Max(topRight.Y, Max(bottomLeft.Y,
        bottomRight.Y)));

    return FloatRect(minX, minY, maxX - minX, maxY - minY);
}


IntRect Matrix::MapBoundingRect(const IntRect &rect) const
{
    const FloatRect r = Map(rect);

    return r.ToExpandedIntRect();
}


bool Matrix::IsIdentity() const
{
    // Look at diagonal elements first to return if scale is not 1.
    return
        m[0][0] == 1 and
        m[1][1] == 1 and
        m[0][1] == 0 and
        m[1][0] == 0 and
        m[2][0] == 0 and
        m[2][1] == 0;
}


bool Matrix::IsEqual(const Matrix &matrix) const
{
    return
        FuzzyIsEqual(m[0][0], matrix.m[0][0]) and
        FuzzyIsEqual(m[0][1], matrix.m[0][1]) and
        FuzzyIsEqual(m[1][0], matrix.m[1][0]) and
        FuzzyIsEqual(m[1][1], matrix.m[1][1]) and
        FuzzyIsEqual(m[2][0], matrix.m[2][0]) and
        FuzzyIsEqual(m[2][1], matrix.m[2][1]);
}


void Matrix::PreTranslate(const FloatPoint &translation) {
    PreMultiply(Matrix(translation));
}


void Matrix::PostTranslate(const FloatPoint &translation) {
    PostMultiply(Matrix(translation));
}


void Matrix::PreTranslate(const double x, const double y) {
    PreTranslate(FloatPoint {
        x, y
    });
}


void Matrix::PostTranslate(const double x, const double y) {
    PostTranslate(FloatPoint {
        x, y
    });
}


void Matrix::PreScale(const FloatPoint &scale) {
    PreMultiply(CreateScale(scale));
}


void Matrix::PostScale(const FloatPoint &scale) {
    PostMultiply(CreateScale(scale));
}


void Matrix::PreScale(const double x, const double y) {
    PreScale(FloatPoint {
        x, y
    });
}


void Matrix::PostScale(const double x, const double y) {
    PostScale(FloatPoint {
        x, y
    });
}


void Matrix::PreScale(const double scale) {
    PreScale(FloatPoint {
        scale, scale
    });
}


void Matrix::PostScale(const double scale) {
    PostScale(FloatPoint {
        scale, scale
    });
}


void Matrix::PreRotate(const double degrees) {
    PreMultiply(CreateRotation(degrees));
}


void Matrix::PostRotate(const double degrees) {
    PostMultiply(CreateRotation(degrees));
}


void Matrix::PostMultiply(const Matrix &matrix)
{
    const double m00 = m[0][0];
    const double m01 = m[0][1];
    const double m10 = m[1][0];
    const double m11 = m[1][1];

    m[0][0] = matrix.m[0][0] * m00 + matrix.m[0][1] * m10;
    m[0][1] = matrix.m[0][0] * m01 + matrix.m[0][1] * m11;
    m[1][0] = matrix.m[1][0] * m00 + matrix.m[1][1] * m10;
    m[1][1] = matrix.m[1][0] * m01 + matrix.m[1][1] * m11;
    m[2][0] = matrix.m[2][0] * m00 + matrix.m[2][1] * m10 + m[2][0];
    m[2][1] = matrix.m[2][0] * m01 + matrix.m[2][1] * m11 + m[2][1];
}


void Matrix::PreMultiply(const Matrix &matrix)
{
    const double m00 = m[0][0];
    const double m01 = m[0][1];
    const double m10 = m[1][0];
    const double m11 = m[1][1];

    m[0][0] = m00 * matrix.m[0][0] + m01 * matrix.m[1][0];
    m[0][1] = m00 * matrix.m[0][1] + m01 * matrix.m[1][1];
    m[1][0] = m10 * matrix.m[0][0] + m11 * matrix.m[1][0];
    m[1][1] = m10 * matrix.m[0][1] + m11 * matrix.m[1][1];
    m[2][0] = m[2][0] * matrix.m[0][0] + m[2][1] * matrix.m[1][0] + matrix.m[2][0];
    m[2][1] = m[2][0] * matrix.m[0][1] + m[2][1] * matrix.m[1][1] + matrix.m[2][1];
}


MatrixComplexity Matrix::DetermineComplexity() const
{
    const bool m00 = FuzzyNotEqual(m[0][0], 1.0);
    const bool m01 = FuzzyNotZero(m[0][1]);
    const bool m10 = FuzzyNotZero(m[1][0]);
    const bool m11 = FuzzyNotEqual(m[1][1], 1.0);
    const bool m20 = FuzzyNotZero(m[2][0]);
    const bool m21 = FuzzyNotZero(m[2][1]);

    const bool translation = m20 | m21;
    const bool scale = m00 | m11;
    const bool complex = m01 | m10;

    static constexpr int TranslationBit = 2;
    static constexpr int ScaleBit = 1;
    static constexpr int ComplexBit = 0;

    const int mask =
        (int(translation) << 2) |
        (int(scale) << 1) |
        (int(complex) << ComplexBit);

    switch (mask) {
        case 0:
            return MatrixComplexity::Identity;
        case (1 << TranslationBit):
            return MatrixComplexity::TranslationOnly;
        case (1 << ScaleBit):
            return MatrixComplexity::ScaleOnly;
        case ((1 << TranslationBit) | (1 << ScaleBit)):
            return MatrixComplexity::TranslationScale;
        default:
            break;
    }

    return MatrixComplexity::Complex;
}
