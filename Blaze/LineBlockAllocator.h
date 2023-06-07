
#pragma once


#include "LineArrayTiled.h"
#include "LineArrayX16Y16.h"
#include "LineArrayX32Y16.h"
#include <new>
#include "Utils.h"


class LineBlockAllocator final {
public:

    LineBlockAllocator() {
    }


   ~LineBlockAllocator();


    /**
     * Returns new tiled line array block. Returned memory is not zero-filled.
     */
    LineArrayTiledBlock *NewTiledBlock(LineArrayTiledBlock *next);


    /**
     * Returns new narrow line array block. Returned memory is not
     * zero-filled.
     */
    LineArrayX16Y16::Block *NewX16Y16Block(LineArrayX16Y16::Block *next);


    /**
     * Returns new wide line array block. Returned memory is not zero-filled.
     */
    LineArrayX32Y16::Block *NewX32Y16Block(LineArrayX32Y16::Block *next);


    /**
     * Resets this allocator to initial state. Should be called
     * after frame ends.
     */
    void Clear();

private:

    // If these get bigger, there is probably too much wasted
    // memory for most input paths.
    STATIC_ASSERT(SIZE_OF(LineArrayX16Y16::Block) <= 1024);
    STATIC_ASSERT(SIZE_OF(LineArrayX32Y16::Block) <= 1024);

    // Points to the current arena.
    uint8 *mCurrent = nullptr;
    uint8 *mEnd = nullptr;

    struct Arena final {
        // Each arena is 32 kilobytes.
        static constexpr int Size = 1024 * 32;

        union {
            uint8 Memory[Size];

            struct {
                // Points to the next item in free list.
                Arena *NextFree;

                // Points to the next item in all block list.
                Arena *NextAll;
            } Links;
        };
    };

    STATIC_ASSERT(SIZE_OF(Arena) == Arena::Size);
    STATIC_ASSERT(SIZE_OF(Arena::Links) == (SIZE_OF(void *) * 2));

    Arena *mAllArenas = nullptr;
    Arena *mFreeArenas = nullptr;

private:
    template <typename T>
    T *NewBlock(T *next);

    template <typename T>
    T *NewBlockFromNewArena(T *next);
private:
    void NewArena();
private:
    DISABLE_COPY_AND_ASSIGN(LineBlockAllocator);
};


FORCE_INLINE LineArrayTiledBlock *LineBlockAllocator::NewTiledBlock(LineArrayTiledBlock *next) {
    return NewBlock<LineArrayTiledBlock>(next);
}


FORCE_INLINE LineArrayX16Y16::Block *LineBlockAllocator::NewX16Y16Block(LineArrayX16Y16::Block *next) {
    return NewBlock<LineArrayX16Y16::Block>(next);
}


FORCE_INLINE LineArrayX32Y16::Block *LineBlockAllocator::NewX32Y16Block(LineArrayX32Y16::Block *next) {
    return NewBlock<LineArrayX32Y16::Block>(next);
}


template <typename T>
FORCE_INLINE T *LineBlockAllocator::NewBlock(T *next) {
    uint8 *current = mCurrent;

    if (LIKELY(current < mEnd)) {
        T *b = reinterpret_cast<T *>(current);

        mCurrent = reinterpret_cast<uint8 *>(b + 1);

        return new (b) T(next);
    }

    return NewBlockFromNewArena<T>(next);
}


template <typename T>
FORCE_INLINE T * LineBlockAllocator::NewBlockFromNewArena(T *next) {
    NewArena();

    T *b = reinterpret_cast<T *>(mCurrent);

    mCurrent = reinterpret_cast<uint8 *>(b + 1);

    return new (b) T(next);
}
