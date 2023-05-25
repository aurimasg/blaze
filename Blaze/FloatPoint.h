
#pragma once


#include "Utils.h"


struct FloatPoint final {
    double X = 0;
    double Y = 0;
};


FORCE_INLINE FloatPoint operator-(const FloatPoint a, const FloatPoint b) {
    return FloatPoint {
        a.X - b.X,
        a.Y - b.Y
    };
}


FORCE_INLINE FloatPoint operator+(const FloatPoint a, const FloatPoint b) {
    return FloatPoint {
        a.X + b.X,
        a.Y + b.Y
    };
}


FORCE_INLINE FloatPoint operator*(const FloatPoint a, const FloatPoint b) {
    return FloatPoint {
        a.X * b.X,
        a.Y * b.Y
    };
}
