#pragma once

#include <stdint.h>
#include <xmmintrin.h>
#include <xmmintrin.h> // SSE
#include <emmintrin.h> // SSE2
#include <pmmintrin.h> // SSE3
#include <tmmintrin.h> // SSSE3
#include <smmintrin.h> // SSE4.1
#include <nmmintrin.h> // SSE4.2

struct xorshift32_state {
    uint32_t a;
};

uint32_t xorshift32(struct xorshift32_state *state);


void xorshift128(__m128i* four_states);
