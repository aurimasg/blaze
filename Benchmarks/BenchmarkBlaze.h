
#pragma once


#include "Benchmark.h"


class BenchmarkBlaze : public Benchmark {
public:
    BenchmarkBlaze();
public:
    virtual void Prepare(const Geometry *geometries, const int geometryCount) override;
    virtual void RenderOnce(const Matrix &matrix, const ImageData &image) override;
private:
    Threads mThreads;
    const Geometry *mGeometries = nullptr;
    int mGeometryCount = 0;
};
