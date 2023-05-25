
#pragma once


#include "Benchmark.h"
#include <CoreGraphics/CoreGraphics.h>


class BenchmarkCoreGraphics : public Benchmark {
public:
    BenchmarkCoreGraphics();
   ~BenchmarkCoreGraphics();
public:
    virtual void Prepare(const Geometry *geometries, const int geometryCount) override;
    virtual void RenderOnce(const Matrix &matrix, const ImageData &image) override;
private:
    CGPathRef *mPaths = nullptr;
    CGColorRef *mColors = nullptr;
    int mGeometryCount = 0;
};
