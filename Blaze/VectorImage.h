
#pragma once


#include "Geometry.h"
#include "IntRect.h"
#include "Utils.h"


/**
 * Parser and maintainer of vector image.
 */
class VectorImage final {
public:
    VectorImage();
   ~VectorImage();
public:
    void Parse(const uint8 *binary, const uint64 length);
    int GetGeometryCount() const;
    IntRect GetBounds() const;
    const Geometry *GetGeometryAt(const int index) const;
    const Geometry *GetGeometries() const;
private:
    void Free();
private:
    int mGeometryCount = 0;
    IntRect mBounds;
    Geometry *mGeometries = nullptr;
private:
    DISABLE_COPY_AND_ASSIGN(VectorImage);
};


FORCE_INLINE int VectorImage::GetGeometryCount() const {
    return mGeometryCount;
}


FORCE_INLINE IntRect VectorImage::GetBounds() const {
    return mBounds;
}


FORCE_INLINE const Geometry *VectorImage::GetGeometryAt(const int index) const {
    ASSERT(index >= 0);
    ASSERT(index < mGeometryCount);

    return mGeometries + index;
}


FORCE_INLINE const Geometry *VectorImage::GetGeometries() const {
    return mGeometries;
}
