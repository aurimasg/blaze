
#pragma once


#include "IntRect.h"


struct FloatRect final {
    constexpr FloatRect(const double x, const double y, const double width,
        const double height)
    :   MinX(x),
        MinY(y),
        MaxX(x + width),
        MaxY(y + height)
    {
    }


    constexpr FloatRect(const IntRect &r)
    :   MinX(r.MinX),
        MinY(r.MinY),
        MaxX(r.MaxX),
        MaxY(r.MaxY)
    {
    }


    IntRect ToExpandedIntRect() const;


    double MinX = 0;
    double MinY = 0;
    double MaxX = 0;
    double MaxY = 0;
};
