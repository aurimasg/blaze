
#pragma once


#include "F24Dot8.h"
#include "FloatPoint.h"


struct F24Dot8Point final {
    F24Dot8 X;
    F24Dot8 Y;
};


static constexpr F24Dot8Point FloatPointToF24Dot8Point(const FloatPoint p) {
    return F24Dot8Point {
        DoubleToF24Dot8(p.X),
        DoubleToF24Dot8(p.Y)
    };
}


static constexpr F24Dot8Point FloatPointToF24Dot8Point(const double x, const double y) {
    return F24Dot8Point {
        DoubleToF24Dot8(x),
        DoubleToF24Dot8(y)
    };
}


STATIC_ASSERT(SIZE_OF(F24Dot8Point) == 8);
