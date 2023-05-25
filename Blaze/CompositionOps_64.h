
// This must only be included from CompositionOps.h


static uint32 ApplyAlpha(const uint32 x, const uint32 a) {
    const uint64 a0 = (((uint64(x)) | ((uint64(x)) << 24)) & 0x00ff00ff00ff00ff) * a;
    const uint64 a1 = (a0 + ((a0 >> 8) & 0xff00ff00ff00ff) + 0x80008000800080) >> 8;
    const uint64 a2 = a1 & 0x00ff00ff00ff00ff;

    return (uint32(a2)) | (uint32(a2 >> 24));
}
