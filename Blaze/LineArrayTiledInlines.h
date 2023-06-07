
#pragma once


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::Construct(LineArrayTiled<T> *placement,
    const int count, ThreadMemory &memory)
{
    ASSERT(placement != nullptr);
    ASSERT(count > 0);

    LineArrayTiled<T> *p = static_cast<LineArrayTiled<T> *>(placement);

    for (int i = 0; i < count; i++) {
        new (p + i) LineArrayTiled<T>();
    }
}


template <typename T>
FORCE_INLINE LineArrayTiledBlock *LineArrayTiled<T>::GetFrontBlock() const {
    return mCurrent;
}


template <typename T>
FORCE_INLINE int LineArrayTiled<T>::GetFrontBlockLineCount() const {
    return mCount;
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendVerticalLine(ThreadMemory &memory, const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1) {
    const TileIndex columnIndex = T::F24Dot8ToTileColumnIndex(x - FindTileColumnAdjustment(x));
    const F24Dot8 ex = x - T::TileColumnIndexToF24Dot8(columnIndex);

    Append(memory, columnIndex, PackF24Dot8ToF8Dot8x2(y0, y1),
        PackF24Dot8ToF8Dot8x2(ex, ex));
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineDownR_V(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    if (LIKELY(p0x < p1x)) {
        AppendLineDownR(memory, p0x, p0y, p1x, p1y);
    } else {
        AppendVerticalLine(memory, p0x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineUpR_V(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    if (LIKELY(p0x < p1x)) {
        AppendLineUpR(memory, p0x, p0y, p1x, p1y);
    } else {
        AppendVerticalLine(memory, p0x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineDownL_V(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    if (LIKELY(p0x > p1x)) {
        AppendLineDownL(memory, p0x, p0y, p1x, p1y);
    } else {
        AppendVerticalLine(memory, p0x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineUpL_V(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    if (LIKELY(p0x > p1x)) {
        AppendLineUpL(memory, p0x, p0y, p1x, p1y);
    } else {
        AppendVerticalLine(memory, p0x, p0y, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineDownRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    ASSERT(x0 != x1);

    if (x0 < x1) {
        AppendLineDownR(memory, x0, y0, x1, y1);
    } else {
        AppendLineDownL(memory, x0, y0, x1, y1);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineUpRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    ASSERT(x0 != x1);

    if (x0 < x1) {
        AppendLineUpR(memory, x0, y0, x1, y1);
    } else {
        AppendLineUpL(memory, x0, y0, x1, y1);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineDownR(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    const TileIndex columnIndex0 = T::F24Dot8ToTileColumnIndex(p0x);
    const TileIndex columnIndex1 = T::F24Dot8ToTileColumnIndex(p1x - 1);

    ASSERT(columnIndex0 <= columnIndex1);

    if (columnIndex0 == columnIndex1) {
        Append(memory, columnIndex0, p0x, p0y, p1x, p1y);
    } else {
        // Number of pixels + 24.8 fraction for start and end points in coordinate
        // system of tiles these points belong to.
        const F24Dot8 fx = p0x - T::TileColumnIndexToF24Dot8(columnIndex0);

        ASSERT(fx >= 0);
        ASSERT(fx <= T::TileWF24Dot8);

        // Horizontal and vertical deltas.
        const F24Dot8 dx = p1x - p0x;
        const F24Dot8 dy = p1y - p0y;

        const F24Dot8 pp = (T::TileWF24Dot8 - fx) * dy;

        F24Dot8 cy = p0y + (pp / dx);

        TileIndex idx = columnIndex0 + 1;

        F24Dot8 cursor = T::TileColumnIndexToF24Dot8(idx);

        Append(memory, columnIndex0, p0x, p0y, cursor, cy);

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = T::TileWF24Dot8 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx++) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy + delta;
                const F24Dot8 nx = cursor + T::TileWF24Dot8;

                Append(memory, idx, cursor, cy, nx, ny);

                cy = ny;
                cursor = nx;
            }
        }

        Append(memory, columnIndex1, cursor, cy, p1x, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineUpR(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x <= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    const TileIndex columnIndex0 = T::F24Dot8ToTileColumnIndex(p0x);
    const TileIndex columnIndex1 = T::F24Dot8ToTileColumnIndex(p1x - 1);

    ASSERT(columnIndex0 <= columnIndex1);

    if (columnIndex0 == columnIndex1) {
        Append(memory, columnIndex0, p0x, p0y, p1x, p1y);
    } else {
        // Number of pixels + 24.8 fraction for start and end points in coordinate
        // system of tiles these points belong to.
        const F24Dot8 fx = p0x - T::TileColumnIndexToF24Dot8(columnIndex0);

        ASSERT(fx >= 0);
        ASSERT(fx <= T::TileWF24Dot8);

        // Horizontal and vertical deltas.
        const F24Dot8 dx = p1x - p0x;
        const F24Dot8 dy = p0y - p1y;

        const F24Dot8 pp = (T::TileWF24Dot8 - fx) * dy;

        F24Dot8 cy = p0y - (pp / dx);

        TileIndex idx = columnIndex0 + 1;

        F24Dot8 cursor = T::TileColumnIndexToF24Dot8(idx);

        Append(memory, columnIndex0, p0x, p0y, cursor, cy);

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = T::TileWF24Dot8 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx++) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy - delta;
                const F24Dot8 nx = cursor + T::TileWF24Dot8;

                Append(memory, idx, cursor, cy, nx, ny);

                cy = ny;
                cursor = nx;
            }
        }

        Append(memory, columnIndex1, cursor, cy, p1x, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineDownL(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y <= p1y);

    const TileIndex columnIndex0 = T::F24Dot8ToTileColumnIndex(p0x - 1);
    const TileIndex columnIndex1 = T::F24Dot8ToTileColumnIndex(p1x);

    ASSERT(columnIndex1 <= columnIndex0);

    if (columnIndex0 == columnIndex1) {
        Append(memory, columnIndex0, p0x, p0y, p1x, p1y);
    } else {
        // Number of pixels + 24.8 fraction for start and end points in coordinate
        // system of tiles these points belong to.
        const F24Dot8 fx = p0x - T::TileColumnIndexToF24Dot8(columnIndex0);

        ASSERT(fx >= 0);
        ASSERT(fx <= T::TileWF24Dot8);

        // Horizontal and vertical deltas.
        const F24Dot8 dx = p0x - p1x;
        const F24Dot8 dy = p1y - p0y;

        const F24Dot8 pp = fx * dy;

        F24Dot8 cy = p0y + (pp / dx);

        TileIndex idx = columnIndex0 - 1;

        F24Dot8 cursor = T::TileColumnIndexToF24Dot8(columnIndex0);

        Append(memory, columnIndex0, p0x, p0y, cursor, cy);

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = T::TileWF24Dot8 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx--) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy + delta;
                const F24Dot8 nx = cursor - T::TileWF24Dot8;

                Append(memory, idx, cursor, cy, nx, ny);

                cy = ny;
                cursor = nx;
            }
        }

        Append(memory, columnIndex1, cursor, cy, p1x, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::AppendLineUpL(ThreadMemory &memory,
    const F24Dot8 p0x, const F24Dot8 p0y, const F24Dot8 p1x,
    const F24Dot8 p1y)
{
    ASSERT(p0x >= p1x);
    ASSERT(p0y >= 0);
    ASSERT(p0y <= T::TileHF24Dot8);
    ASSERT(p1y >= 0);
    ASSERT(p1y <= T::TileHF24Dot8);
    ASSERT(p0y >= p1y);

    const TileIndex columnIndex0 = T::F24Dot8ToTileColumnIndex(p0x - 1);
    const TileIndex columnIndex1 = T::F24Dot8ToTileColumnIndex(p1x);

    ASSERT(columnIndex1 <= columnIndex0);

    if (columnIndex0 == columnIndex1) {
        Append(memory, columnIndex0, p0x, p0y, p1x, p1y);
    } else {
        // Number of pixels + 24.8 fraction for start and end points in coordinate
        // system of tiles these points belong to.
        const F24Dot8 fx = p0x - T::TileColumnIndexToF24Dot8(columnIndex0);

        ASSERT(fx >= 0);
        ASSERT(fx <= T::TileWF24Dot8);

        // Horizontal and vertical deltas.
        const F24Dot8 dx = p0x - p1x;
        const F24Dot8 dy = p0y - p1y;

        const F24Dot8 pp = fx * dy;

        F24Dot8 cy = p0y - (pp / dx);

        TileIndex idx = columnIndex0 - 1;

        F24Dot8 cursor = T::TileColumnIndexToF24Dot8(columnIndex0);

        Append(memory, columnIndex0, p0x, p0y, cursor, cy);

        if (idx != columnIndex1) {
            F24Dot8 mod = (pp % dx) - dx;

            const F24Dot8 p = T::TileWF24Dot8 * dy;
            const F24Dot8 lift = p / dx;
            const F24Dot8 rem = p % dx;

            for ( ; idx != columnIndex1; idx--) {
                F24Dot8 delta = lift;

                mod += rem;

                if (mod >= 0) {
                    mod -= dx;
                    delta++;
                }

                const F24Dot8 ny = cy - delta;
                const F24Dot8 nx = cursor - T::TileWF24Dot8;

                Append(memory, idx, cursor, cy, nx, ny);

                cy = ny;
                cursor = nx;
            }
        }

        Append(memory, columnIndex1, cursor, cy, p1x, p1y);
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::Append(ThreadMemory &memory,
    const TileIndex columnIndex, const F24Dot8 x0, const F24Dot8 y0,
    const F24Dot8 x1, const F24Dot8 y1)
{
    if (y0 != y1) {
        const F24Dot8 cx = T::TileColumnIndexToF24Dot8(columnIndex);
        const F24Dot8 ex0 = x0 - cx;
        const F24Dot8 ex1 = x1 - cx;

        Append(memory, columnIndex, PackF24Dot8ToF8Dot8x2(y0, y1),
            PackF24Dot8ToF8Dot8x2(ex0, ex1));
    }
}


template <typename T>
FORCE_INLINE void LineArrayTiled<T>::Append(ThreadMemory &memory,
    const TileIndex columnIndex, const F8Dot8x2 y0y1, const F8Dot8x2 x0x1)
{
    LineArrayTiledBlock *current = mCurrent;
    const int count = mCount;

    if (count < LineArrayTiledBlock::LinesPerBlock) {
        // Most common.
        current->Y0Y1[count] = y0y1;
        current->X0X1[count] = x0x1;
        current->Indices[count] = columnIndex;

        mCount = count + 1;
    } else {
        LineArrayTiledBlock *b = memory.FrameNewTiledBlock(current);

        b->Y0Y1[0] = y0y1;
        b->X0X1[0] = x0x1;
        b->Indices[0] = columnIndex;

        // Set count to 1 for segment being added.
        mCount = 1;

        mCurrent = b;
    }
}
