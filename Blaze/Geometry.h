
#pragma once


#include "FillRule.h"
#include "FloatPoint.h"
#include "IntRect.h"
#include "Matrix.h"
#include "PathTag.h"


/**
 * One renderable item.
 */
struct Geometry final {

    /**
     * Constructs geometry.
     *
     * @param pathBounds Bounding box of a path transformed by transformation
     * matrix. This rectangle potentially can exceed bounds of destination
     * image.
     *
     * @param tags Pointer to tags. Must not be nullptr.
     *
     * @param points Pointer to points. Must not be nullptr.
     *
     * @param tm Transformation matrix.
     *
     * @param tagCount A number of tags. Must be greater than 0.
     *
     * @param color RGBA color, 8 bits per channel, color components
     * premultiplied by alpha.
     *
     * @param rule Fill rule to use.
     */
    Geometry(const IntRect &pathBounds, const PathTag *tags,
        const FloatPoint *points, const Matrix &tm, const int tagCount,
        const int pointCount, const uint32 color, const FillRule rule);


    const IntRect PathBounds;
    const PathTag *Tags = nullptr;
    const FloatPoint *Points = nullptr;
    const Matrix TM;
    const int TagCount = 0;
    const int PointCount = 0;
    const uint32 Color = 0;
    const FillRule Rule = FillRule::NonZero;
};
