
#pragma once


#include "F8Dot8.h"
#include "TileBounds.h"


class ThreadMemory;


template <typename T>
struct LineArrayTiled final {
    struct Block final {
        constexpr explicit Block(Block *next)
        :   Next(next)
        {
        }


        static constexpr int LinesPerBlock = 32;


        // Y0 and Y1 encoded as two 8.8 fixed point numbers packed into one 32 bit
        // integer.
        F8Dot8x2 Y0Y1[LinesPerBlock];
        F8Dot8x2 X0X1[LinesPerBlock];
        TileIndex Indices[LinesPerBlock];

        // Pointer to the next block of lines in the same row.
        Block *Next = nullptr;
    private:
        Block() = delete;
    private:
        DISABLE_COPY_AND_ASSIGN(Block);
    };

public:

    void AppendVerticalLine(ThreadMemory &memory, const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1);
    void AppendLineDownR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineDownL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineDownRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);

private:
    void AppendLineDownR(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpR(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineDownL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
private:
    void Append(ThreadMemory &memory, const TileIndex columnIndex,
        const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void Append(ThreadMemory &memory, const TileIndex columnIndex,
        const F8Dot8x2 y0y1, const F8Dot8x2 x0x1);
private:

    static constexpr int AdjustmentMask = (F24Dot8_1 * T::TileW) - 1;

    static constexpr int FindTileColumnAdjustment(const F24Dot8 value) {
        STATIC_ASSERT(SIZE_OF(F24Dot8) == 4);

        // Will be set to 0 is value is zero or less. Otherwise it will be 1.
        const int lte0 = ~((value - 1) >> 31) & 1;

        // Will be set to 1 if value is divisible by tile width (in 24.8
        // format) without a reminder. Otherwise it will be 0.
        const int db = (((value & AdjustmentMask) - 1) >> 31) & 1;

        // Return 1 if both bits (more than zero and disisible by 256) are set.
        return lte0 & db;
    }

private:
    Block *mCurrent = nullptr;
    int mCount = Block::LinesPerBlock;
};
