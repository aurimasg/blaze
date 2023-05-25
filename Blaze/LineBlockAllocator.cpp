
#include "LineBlockAllocator.h"


LineBlockAllocator::~LineBlockAllocator()
{
    Arena *p = mAllArenas;

    while (p != nullptr) {
        Arena *next = p->Links.NextAll;

        free(p);

        p = next;
    }
}


void LineBlockAllocator::Clear()
{
    Arena *l = nullptr;

    Arena *p = mAllArenas;

    while (p != nullptr) {
        Arena *next = p->Links.NextAll;

        p->Links.NextFree = l;

        l = p;

        p = next;
    }

    mCurrent = nullptr;
    mEnd = nullptr;
    mFreeArenas = l;
}


void LineBlockAllocator::NewArena()
{
    Arena *p = mFreeArenas;

    if (p != nullptr) {
        mFreeArenas = p->Links.NextFree;
    } else {
        p = static_cast<Arena *>(malloc(SIZE_OF(Arena)));

        p->Links.NextAll = mAllArenas;

        mAllArenas = p;
    }

    p->Links.NextFree = nullptr;

    mCurrent = p->Memory + SIZE_OF(Arena::Links);
    mEnd = p->Memory + Arena::Size -
        SIZE_OF(LineArrayX32Y16::Block);
}
