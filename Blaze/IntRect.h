
#pragma once


struct IntRect final {
    constexpr IntRect(const int x, const int y, const int width,
        const int height)
    :   MinX(x),
        MinY(y),
        MaxX(x + width),
        MaxY(y + height)
    {
    }

    int MinX = 0;
    int MinY = 0;
    int MaxX = 0;
    int MaxY = 0;
};
