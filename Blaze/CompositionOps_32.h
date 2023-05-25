
// This must only be included from CompositionOps.h


static uint32 ApplyAlpha(const uint32 x, const uint32 a) {
    const uint32 a0 = (x & 0x00ff00ff) * a;
    const uint32 a1 = (a0 + ((a0 >> 8) & 0x00ff00ff) + 0x00800080) >> 8;
    const uint32 a2 = a1 & 0x00ff00ff;

    const uint32 b0 = ((x >> 8) & 0x00ff00ff) * a;
    const uint32 b1 = (b0 + ((b0 >> 8) & 0x00ff00ff) + 0x00800080);
    const uint32 b2 = b1 & 0xff00ff00;

    return a2 | b2;
}
