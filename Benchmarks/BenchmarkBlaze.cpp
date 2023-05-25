
#include "BenchmarkBlaze.h"


BenchmarkBlaze::BenchmarkBlaze() {
}


void BenchmarkBlaze::Prepare(const Geometry *geometries,
    const int geometryCount)
{
    mGeometries = geometries;
    mGeometryCount = geometryCount;
}


void BenchmarkBlaze::RenderOnce(const Matrix &matrix, const ImageData &image)
{
    Rasterize<TileDescriptor_8x16>(mGeometries, mGeometryCount, matrix, mThreads, image);

    // Free all the memory allocated by threads.
    mThreads.ResetFrameMemory();
}
