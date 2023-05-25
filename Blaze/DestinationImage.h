
#pragma once


#include "VectorImage.h"
#include "Geometry.h"
#include "ImageData.h"
#include "IntSize.h"
#include "Linearizer.h"
#include "Rasterizer.h"
#include "Threads.h"
#include "Utils.h"


/**
 * A helper class for managing an image to draw on.
 */
template <typename T>
class DestinationImage final {
public:

    DestinationImage() {
    }

   ~DestinationImage();

public:

    IntSize UpdateSize(const IntSize &size);

    void ClearImage();

    void DrawImage(const VectorImage &image, const Matrix &matrix);

    IntSize GetImageSize() const;
    int GetImageWidth() const;
    int GetImageHeight() const;
    uint8 *GetImageData() const;
    int GetBytesPerRow() const;

private:
    uint8 *mImageData = nullptr;
    IntSize mImageSize;
    int mBytesPerRow = 0;
    int mImageDataSize = 0;
    Threads mThreads;
private:
    DISABLE_COPY_AND_ASSIGN(DestinationImage);
};


template <typename T>
FORCE_INLINE DestinationImage<T>::~DestinationImage() {
    free(mImageData);
}


template <typename T>
FORCE_INLINE IntSize DestinationImage<T>::UpdateSize(const IntSize &size) {
    ASSERT(size.Width > 0);
    ASSERT(size.Height > 0);

    // Round-up width to tile width.
    const TileIndex w = CalculateColumnCount<T>(size.Width) * T::TileW;

    // Calculate how many bytes are required for the image.
    const int bytes = w * 4 * size.Height;

    if (mImageDataSize < bytes) {
        static constexpr int ImageSizeRounding = 1024 * 32;
        static constexpr int ImageSizeRoundingMask = ImageSizeRounding - 1;

        const int m = bytes + ImageSizeRoundingMask;

        const int bytesRounded = m & ~ImageSizeRoundingMask;

        free(mImageData);

        mImageData = static_cast<uint8 *>(malloc(bytesRounded));
        mImageDataSize = bytesRounded;
    }

    mImageSize.Width = w;
    mImageSize.Height = size.Height;
    mBytesPerRow = w * 4;

    return mImageSize;
}


template <typename T>
FORCE_INLINE void DestinationImage<T>::ClearImage() {
    memset(mImageData, 0, mImageSize.Width * 4 * mImageSize.Height);
}


template <typename T>
FORCE_INLINE void DestinationImage<T>::DrawImage(const VectorImage &image, const Matrix &matrix) {
    if (image.GetGeometryCount() < 1) {
        return;
    }

    const ImageData d(mImageData, mImageSize.Width, mImageSize.Height,
        mBytesPerRow);

    Rasterize<T>(image.GetGeometries(), image.GetGeometryCount(), matrix,
        mThreads, d);

    // Free all the memory allocated by threads.
    mThreads.ResetFrameMemory();
}


template <typename T>
FORCE_INLINE IntSize DestinationImage<T>::GetImageSize() const {
    return mImageSize;
}


template <typename T>
FORCE_INLINE int DestinationImage<T>::GetImageWidth() const {
    return mImageSize.Width;
}


template <typename T>
FORCE_INLINE int DestinationImage<T>::GetImageHeight() const {
    return mImageSize.Height;
}


template <typename T>
FORCE_INLINE uint8 *DestinationImage<T>::GetImageData() const {
    return mImageData;
}


template <typename T>
FORCE_INLINE int DestinationImage<T>::GetBytesPerRow() const {
    return mBytesPerRow;
}
