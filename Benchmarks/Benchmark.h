
#pragma once


#include <Blaze/Blaze.h>


class Benchmark {
public:
    virtual ~Benchmark() {
    }
public:
    double Run(const VectorImage &vg, const double scale, const char *op);
public:
    virtual void Prepare(const Geometry *geometries, const int geometryCount) = 0;
    virtual void RenderOnce(const Matrix &matrix, const ImageData &image) = 0;
};
