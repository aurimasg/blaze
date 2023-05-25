
#include "BenchmarkBlend2D.h"


BenchmarkBlend2D::BenchmarkBlend2D()
{
}


void BenchmarkBlend2D::Prepare(const Geometry *geometries,
    const int geometryCount)
{
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

        const uint32 nr = Clamp<uint32>(static_cast<uint32>(Round(r * 255.0)), 0, 255);
        const uint32 ng = Clamp<uint32>(static_cast<uint32>(Round(g * 255.0)), 0, 255);
        const uint32 nb = Clamp<uint32>(static_cast<uint32>(Round(b * 255.0)), 0, 255);
        const uint32 nc = nr | (ng << 8) | (nb << 16) | (color & 0xff000000);

        mColors.append(BLRgba32(nc));

        const PathTag *tags = geometries[i].Tags;
        const FloatPoint *points = geometries[i].Points;
        const int count = geometries[i].TagCount;

        BLPath path;

        for (int i = 0; i < count; i++) {
            switch (tags[i]) {
                case PathTag::Move:
                    path.moveTo(points[0].X, points[0].Y);
                    points++;
                    break;
                case PathTag::Line:
                    path.lineTo(points[0].X, points[0].Y);
                    points++;
                    break;
                case PathTag::Quadratic:
                    path.quadTo(points[0].X, points[0].Y, points[1].X, points[1].Y);
                    points += 2;
                    break;
                case PathTag::Cubic:
                    path.cubicTo(points[0].X, points[0].Y, points[1].X, points[1].Y, points[2].X, points[2].Y);
                    points += 3;
                    break;
                case PathTag::Close:
                    path.close();
                    break;
            }
        }

        mPaths.append(path);
    }
}


void BenchmarkBlend2D::RenderOnce(const Matrix &matrix,
    const ImageData &image)
{
    BLImage destination;

    destination.createFromData(image.Width, image.Height, BL_FORMAT_PRGB32,
        image.Data, image.BytesPerRow);

    BLContextCreateInfo createInfo {};

    createInfo.threadCount = Threads::GetHardwareThreadCount();

    BLContext context(destination, createInfo);

    const BLMatrix2D m(matrix.M11(), matrix.M12(), matrix.M21(), matrix.M22(),
        matrix.M31(), matrix.M32());

    context.setMatrix(m);
    context.setCompOp(BL_COMP_OP_SRC_OVER);

    for (int i = 0; i < mPaths.size(); i++) {
        context.setFillStyle(mColors[i]);
        context.fillPath(mPaths[i]);
    }

    context.end();
}
