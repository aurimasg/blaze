
#pragma once


#include "Utils.h"


#ifdef MACHINE_64
#include "CompositionOps_64.h"
#elif defined MACHINE_32
#include "CompositionOps_32.h"
#else
#error I don't know about register size.
#endif


static FORCE_INLINE uint32 BlendSourceOver(const uint32 d, const uint32 s) {
    return s + ApplyAlpha(d, 255 - (s >> 24));
}


static FORCE_INLINE void CompositeSpanSourceOver(const int pos, const int end, uint32 *d, const int32 alpha, const uint32 color) {
    ASSERT(pos >= 0);
    ASSERT(pos < end);
    ASSERT(d != nullptr);
    ASSERT(alpha <= 255);

    // For opaque colors, use opaque span composition version.
    ASSERT((color >> 24) < 255);

    const int e = end;
    const uint32 cba = ApplyAlpha(color, alpha);

    for (int x = pos; x < e; x++) {
        const uint32 dd = d[x];

        if (dd == 0) {
            d[x] = cba;
        } else {
            d[x] = BlendSourceOver(dd, cba);
        }
    }
}


static FORCE_INLINE void CompositeSpanSourceOverOpaque(const int pos, const int end, uint32 *d, const int32 alpha, const uint32 color) {
    ASSERT(pos >= 0);
    ASSERT(pos < end);
    ASSERT(d != nullptr);
    ASSERT(alpha <= 255);
    ASSERT((color >> 24) == 255);

    const int e = end;

    if (alpha == 255) {
        // Solid span, write only.
        for (int x = pos; x < e; x++) {
            d[x] = color;
        }
    } else {
        // Transparent span.
        const uint32 cba = ApplyAlpha(color, alpha);

        for (int x = pos; x < e; x++) {
            const uint32 dd = d[x];

            if (dd == 0) {
                d[x] = cba;
            } else {
                d[x] = BlendSourceOver(dd, cba);
            }
        }
    }
}


struct SpanBlender final {
    constexpr explicit SpanBlender(const uint32 color)
    :   Color(color)
    {
    }


    void CompositeSpan(const int pos, const int end, uint32 *d, const int32 alpha) const;

    const uint32 Color = 0;
};


FORCE_INLINE void SpanBlender::CompositeSpan(const int pos, const int end, uint32 *d, const int32 alpha) const {
    CompositeSpanSourceOver(pos, end, d, alpha, Color);
}


STATIC_ASSERT(SIZE_OF(SpanBlender) == 4);


/**
 * Span blender which assumes source color is opaque.
 */
struct SpanBlenderOpaque final {
    constexpr explicit SpanBlenderOpaque(const uint32 color)
    :   Color(color)
    {
    }


    void CompositeSpan(const int pos, const int end, uint32 *d, const int32 alpha) const;

    const uint32 Color = 0;
};


FORCE_INLINE void SpanBlenderOpaque::CompositeSpan(const int pos, const int end, uint32 *d, const int32 alpha) const {
    CompositeSpanSourceOverOpaque(pos, end, d, alpha, Color);
}


STATIC_ASSERT(SIZE_OF(SpanBlenderOpaque) == 4);
