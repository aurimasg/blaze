
#pragma once


#include "Utils.h"


/**
 * Bézier path command.
 */
enum class PathTag : uint8 {
    Move = 0,
    Line,
    Quadratic,
    Cubic,
    Close
};
