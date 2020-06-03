/* 06.05.2020 - Sydney Hauke - HPC - REDS - HEIG-VD */

#pragma once

#include <stddef.h>
#include <xmmintrin.h>
#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3
#include <tmmintrin.h> // SSSE3
#include <smmintrin.h> // SSE4.1
#include <nmmintrin.h> // SSE4.2

void ifs(char *pathname, size_t passes, size_t min_width);
