
#pragma once


#include "Benchmark.h"
#include <blend2d.h>


class BenchmarkBlend2D : public Benchmark {
public:
    BenchmarkBlend2D();
public:
    virtual void Prepare(const Geometry *geometries, const int geometryCount) override;
    virtual void RenderOnce(const Matrix &matrix, const ImageData &image) override;
private:
    BLArray<BLPath> mPaths;
    BLArray<BLRgba> mColors;
};
