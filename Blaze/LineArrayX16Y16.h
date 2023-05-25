
#pragma once


#include "F8Dot8.h"
#include "F24Dot8.h"


class ThreadMemory;


struct LineArrayX16Y16 final {
    LineArrayX16Y16() {
    }


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

        // Pointer to the next block of lines in the same row.
        Block *Next = nullptr;
    private:
        Block() = delete;
    private:
        DISABLE_COPY_AND_ASSIGN(Block);
    };


    Block *GetFrontBlock() const;
    int GetFrontBlockLineCount() const;
    void AppendVerticalLine(ThreadMemory &memory, const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1);
    void AppendLineDownR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineDownL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineDownRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
    void AppendLineUpRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);

private:
    void AppendLine(ThreadMemory &memory, const F8Dot8x2 y0y1, const F8Dot8x2 x0x1);
    void AppendLine(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1);
private:
    Block *mCurrent = nullptr;
    int mCount = Block::LinesPerBlock;
private:
    DISABLE_COPY_AND_ASSIGN(LineArrayX16Y16);
};
