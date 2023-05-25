
#pragma once


#include <skia.h>
#include <vector>
#include "Benchmark.h"


class BenchmarkSkia : public Benchmark {
public:
    BenchmarkSkia();
public:
    virtual void Prepare(const Geometry *geometries, const int geometryCount) override;
    virtual void RenderOnce(const Matrix &matrix, const ImageData &image) override;
private:
    std::vector<SkPath> mPaths;
    std::vector<SkColor> mColors;
};

