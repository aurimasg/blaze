
#include "FloatRect.h"
#include "Utils.h"


IntRect FloatRect::ToExpandedIntRect() const
{
    const int minx = int(Floor(MinX));
    const int miny = int(Floor(MinY));
    const int maxx = int(Ceil(MaxX));
    const int maxy = int(Ceil(MaxY));

    return IntRect(minx, miny, maxx - minx, maxy - miny);
}
