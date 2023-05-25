
#include "Geometry.h"


Geometry::Geometry(const IntRect &pathBounds, const PathTag *tags,
    const FloatPoint *points, const Matrix &tm, const int tagCount,
    const int pointCount, const uint32 color, const FillRule rule)
:   PathBounds(pathBounds),
    Tags(tags),
    Points(points),
    TM(tm),
    TagCount(tagCount),
    PointCount(pointCount),
    Color(color),
    Rule(rule)
{
    ASSERT(tags != nullptr);
    ASSERT(points != nullptr);
    ASSERT(tagCount > 0);
    ASSERT(pointCount > 0);
}
