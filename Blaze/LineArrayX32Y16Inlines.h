
#pragma once


FORCE_INLINE void LineArrayX32Y16::Construct(LineArrayX32Y16 *placement,
    const int count, ThreadMemory &memory)
{
    ASSERT(placement != nullptr);
    ASSERT(count > 0);

    for (int i = 0; i < count; i++) {
        new (placement + i) LineArrayX32Y16();
    }
}


FORCE_INLINE LineArrayX32Y16::Block *LineArrayX32Y16::GetFrontBlock() const {
    return mCurrent;
}


FORCE_INLINE int LineArrayX32Y16::GetFrontBlockLineCount() const {
    return mCount;
}


FORCE_INLINE void LineArrayX32Y16::AppendVerticalLine(ThreadMemory &memory, const F24Dot8 x, const F24Dot8 y0, const F24Dot8 y1) {
    AppendLine(memory, x, y0, x, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineDownR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineUpR_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineDownL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineUpL_V(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1)  {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineDownRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1)  {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLineUpRL(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1)  {
    AppendLine(memory, x0, y0, x1, y1);
}


FORCE_INLINE void LineArrayX32Y16::AppendLine(ThreadMemory &memory, const F8Dot8x2 y0y1, const F24Dot8 x0, const F24Dot8 x1) {
    Block *current = mCurrent;
    const int count = mCount;

    if (count < Block::LinesPerBlock) {
        // Most common.
        current->Y0Y1[count] = y0y1;
        current->X0[count] = x0;
        current->X1[count] = x1;

        mCount = count + 1;
    } else {
        Block *b = memory.FrameNewX32Y16Block(current);

        b->Y0Y1[0] = y0y1;
        b->X0[0] = x0;
        b->X1[0] = x1;

        // Set count to 1 for segment being added.
        mCount = 1;

        mCurrent = b;
    }
}


FORCE_INLINE void LineArrayX32Y16::AppendLine(ThreadMemory &memory, const F24Dot8 x0, const F24Dot8 y0, const F24Dot8 x1, const F24Dot8 y1) {
    if (y0 != y1) {
        AppendLine(memory, PackF24Dot8ToF8Dot8x2(y0, y1), x0, x1);
    }
}
