
#include "BenchmarkSkia.h"


BenchmarkSkia::BenchmarkSkia()
{
}


void BenchmarkSkia::Prepare(const Geometry *geometries,
    const int geometryCount)
{
    for (int i = 0; i < geometryCount; i++) {
        const uint32 color = geometries[i].Color;

        mColors.push_back(((color & 255) << 16) | (((color >> 8) & 255) << 8) |
            ((color >> 16) & 255) | (color & 0xff000000));

        const PathTag *tags = geometries[i].Tags;
        const FloatPoint *points = geometries[i].Points;
        const int count = geometries[i].TagCount;

        SkPath path;

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

        mPaths.push_back(path);
    }
}


void BenchmarkSkia::RenderOnce(const Matrix &matrix,
    const ImageData &image)
{
    SkImageInfo info = SkImageInfo::MakeN32Premul(image.Width, image.Height);

    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, image.Data,
        image.BytesPerRow);

    const SkScalar affine[6] = {
        SkScalar(matrix.M11()),
        SkScalar(matrix.M12()),
        SkScalar(matrix.M21()),
        SkScalar(matrix.M22()),
        SkScalar(matrix.M31()),
        SkScalar(matrix.M32())
    };

    SkMatrix m;

    m.setAffine(affine);

    canvas->setMatrix(m);

    for (int i = 0; i < mPaths.size(); i++) {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setColor(mColors[i]);
        paint.setStyle(SkPaint::kFill_Style);

        canvas->drawPath(mPaths[i], paint);
    }

    canvas->flush();
}
