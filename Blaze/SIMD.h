
#pragma once


#include "F24Dot8.h"
#include "F24Dot8Point.h"
#include "Matrix.h"


#ifdef SIMD_NEON
#include "SIMD_neon.h"
#else
#include "SIMD_generic.h"
#endif
