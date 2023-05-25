
#include "BenchmarkCoreGraphics.h"


BenchmarkCoreGraphics::BenchmarkCoreGraphics()
{
}


BenchmarkCoreGraphics::~BenchmarkCoreGraphics()
{
    for (int i = 0; i < mGeometryCount; i++) {
        CGPathRelease(mPaths[i]);
        CGColorRelease(mColors[i]);
    }

    delete [] mPaths;
    delete [] mColors;
}


void BenchmarkCoreGraphics::Prepare(const Geometry *geometries,
    const int geometryCount)
{
    mGeometryCount = geometryCount;

    mPaths = new CGPathRef [geometryCount];
    mColors = new CGColorRef [geometryCount];

    for (int i = 0; i < geometryCount; i++) {
        const uint32 color = geometries[i].Color;
        double r = double(color & 255) / 255.0;
        double g = double((color >> 8) & 255) / 255.0;
        double b = double((color >> 16) & 255) / 255.0;
        const double a = double(color >> 24) / 255.0;

        // Blend2D wants unpremultiplied input colors.
        if (a <= DBL_EPSILON) {
            continue;
        }

        if (a < 1.0) {
            const double inv = 1.0 / a;
            r = r * inv;
            g = g * inv;
            b = b * inv;
        }

        CGColorRef c = CGColorCreateSRGB(r, g, b, a);

        mColors[i] = c;

        const PathTag *tags = geometries[i].Tags;
        const FloatPoint *points = geometries[i].Points;
        const int count = geometries[i].TagCount;

        CGMutablePathRef path = CGPathCreateMutable();

        mPaths[i] = path;

        for (int i = 0; i < count; i++) {
            switch (tags[i]) {
                case PathTag::Move:
                    CGPathMoveToPoint(path, nullptr, points[0].X, points[0].Y);
                    points++;
                    break;
                case PathTag::Line:
                    CGPathAddLineToPoint(path, nullptr, points[0].X, points[0].Y);
                    points++;
                    break;
                case PathTag::Quadratic:
                    CGPathAddQuadCurveToPoint(path, nullptr, points[0].X, points[0].Y, points[1].X, points[1].Y);
                    points += 2;
                    break;
                case PathTag::Cubic:
                    CGPathAddCurveToPoint(path, nullptr, points[0].X, points[0].Y, points[1].X, points[1].Y, points[2].X, points[2].Y);
                    points += 3;
                    break;
                case PathTag::Close:
                    CGPathCloseSubpath(path);
                    break;
            }
        }
    }
}


static void FlipQuartzCoordinates(CGContextRef context, const CGFloat height)
{
    const CGAffineTransform currentTransform = CGContextGetCTM(context);
    const CGAffineTransform flipTransform = CGAffineTransformMakeScale(1, -1);
    const CGAffineTransform translationTransform = CGAffineTransformMakeTranslation(0, height);
    const CGAffineTransform newTransform = CGAffineTransformConcat(currentTransform, CGAffineTransformConcat(flipTransform, translationTransform));

    CGContextConcatCTM(context, newTransform);
}


void BenchmarkCoreGraphics::RenderOnce(const Matrix &matrix,
    const ImageData &image)
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    CGContextRef context = CGBitmapContextCreate(image.Data, image.Width,
        image.Height, 8, image.BytesPerRow, colorSpace,
        kCGImageAlphaPremultipliedLast);

    CGColorSpaceRelease(colorSpace);

    FlipQuartzCoordinates(context, image.Height);

    const CGAffineTransform m = CGAffineTransformMake(matrix.M11(),
        matrix.M12(), matrix.M21(), matrix.M22(), matrix.M31(), matrix.M32());

    CGContextConcatCTM(context, m);

    for (int i = 0; i < mGeometryCount; i++) {
        CGContextSaveGState(context);
        CGContextSetFillColorWithColor(context, mColors[i]);
        CGContextAddPath(context, mPaths[i]);
        CGContextFillPath(context);
        CGContextRestoreGState(context);
    }

    CGContextRelease(context);
}
