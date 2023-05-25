
#include "VectorImage.h"
#include <new>


struct BinaryReader final {
    constexpr BinaryReader(const uint8 *binary, const uint64 length)
    :   Bytes(binary),
        End(binary + length)
    {
    }

    const uint8 *Bytes = nullptr;
    const uint8 *End = nullptr;

    uint64 GetRemainingByteCount() const;
    int8 ReadInt8();
    uint8 ReadUInt8();
    int32 ReadInt32();
    uint32 ReadUInt32();
    void ReadBinary(uint8 *d, const uint64 length);
};


FORCE_INLINE uint64 BinaryReader::GetRemainingByteCount() const {
    return static_cast<uint64>(End - Bytes);
}


FORCE_INLINE uint32 BinaryReader::ReadUInt32() {
    const uint64 r = static_cast<uint64>(End - Bytes);

    if (r >= 4) {
        const uint32 n = *reinterpret_cast<const uint32 *>(Bytes);

        Bytes += 4;

        return n;
    }

    return 0;
}


FORCE_INLINE int8 BinaryReader::ReadInt8() {
    if (Bytes < End) {
        const int8 n = *reinterpret_cast<const int8 *>(Bytes);

        Bytes++;

        return n;
    }

    return 0;
}


FORCE_INLINE uint8 BinaryReader::ReadUInt8() {
    if (Bytes < End) {
        const uint8 n = *reinterpret_cast<const uint8 *>(Bytes);

        Bytes++;

        return n;
    }

    return 0;
}


FORCE_INLINE int32 BinaryReader::ReadInt32() {
    const uint64 r = static_cast<uint64>(End - Bytes);

    if (r >= 4) {
        const int32 n = *reinterpret_cast<const int32 *>(Bytes);

        Bytes += 4;

        return n;
    }

    return 0;
}


FORCE_INLINE void BinaryReader::ReadBinary(uint8 *d, const uint64 length) {
    ASSERT(GetRemainingByteCount() >= length);

    memcpy(d, Bytes, length);

    Bytes += length;
}


VectorImage::VectorImage()
:   mBounds(0, 0, 0, 0)
{
}


VectorImage::~VectorImage()
{
    Free();
}


void VectorImage::Parse(const uint8 *binary, const uint64 length)
{
    ASSERT(binary != nullptr);
    ASSERT(length > 0);

    Free();

    BinaryReader br(binary, length);

    // Read signature.
    const uint8 B = br.ReadUInt8();
    const uint8 v = br.ReadUInt8();
    const uint8 e = br.ReadUInt8();
    const uint8 c = br.ReadUInt8();

    if (B != 'B' or v != 'v' or e != 'e' or c != 'c') {
        return;
    }

    // Read version.
    const uint32 version = br.ReadUInt32();

    if (version != 1) {
        return;
    }

    // 4 bytes, total path count.
    const uint32 count = br.ReadUInt32();

    // 16 bytes, full bounds.
    const int iminx = br.ReadInt32();
    const int iminy = br.ReadInt32();
    const int imaxx = br.ReadInt32();
    const int imaxy = br.ReadInt32();

    mBounds = IntRect(iminx, iminy, imaxx - iminx, imaxy - iminy);

    // Each path entry is at least 32 bytes plus 4 bytes indicating path count
    // plus 16 bytes indicating full bounds plus 8 bytes for signature and
    // version.
    if (length < ((count * 32) + 4 + 16 + 8)) {
        // File is smaller than it says it has paths in it.
        return;
    }

    mGeometries = static_cast<Geometry *>(malloc(SIZE_OF(Geometry) * count));

    for (uint32 i = 0; i < count; i++) {
        // 4 bytes, color as premultiplied RGBA8.
        const uint32 color = br.ReadUInt32();

        // 16 bytes, path bounds.
        const int pminx = br.ReadInt32();
        const int pminy = br.ReadInt32();
        const int pmaxx = br.ReadInt32();
        const int pmaxy = br.ReadInt32();

        // 4 bytes, fill rule.
        const FillRule fillRule = static_cast<FillRule>(br.ReadUInt32() & 1);

        // 4 bytes, tag count.
        const uint32 tagCount = br.ReadUInt32();

        // 4 bytes, point count.
        const uint32 pointCount = br.ReadUInt32();

        const uint64 memoryNeeded = tagCount + (pointCount * 16);

        if (br.GetRemainingByteCount() < memoryNeeded) {
            // Less bytes left to read than the file says there are tags and
            // points stored.
            break;
        }

        PathTag *tags = static_cast<PathTag *>(malloc(tagCount));
        FloatPoint *points = static_cast<FloatPoint *>(malloc(pointCount * 16));

        br.ReadBinary(reinterpret_cast<uint8 *>(tags), tagCount);
        br.ReadBinary(reinterpret_cast<uint8 *>(points), pointCount * 16);

        Geometry *geometry = mGeometries + mGeometryCount;

        new (geometry) Geometry(IntRect(pminx, pminy, pmaxx - pminx, pmaxy - pminy),
            tags, points, Matrix::Identity, tagCount, pointCount, color,
            fillRule);

        mGeometryCount++;
    }
}


void VectorImage::Free()
{
    const int count = mGeometryCount;

    for (int i = 0; i < count; i++) {
        free((void *)mGeometries[i].Tags);
        free((void *)mGeometries[i].Points);
    }

    free(mGeometries);

    mGeometries = nullptr;
    mGeometryCount = 0;
}
