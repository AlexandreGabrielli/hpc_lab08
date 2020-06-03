/* 06.05.2020 - Sydney Hauke - HPC - REDS - HEIG-VD */

#include "random.h"



uint32_t xorshift32(struct xorshift32_state *state)
{
    uint32_t x = state->a;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state->a = x;
}


void xorshift128(__m128i* four_states){
    
    __m128i resultat = *four_states;

    __m128i r1 = _mm_slli_epi32(resultat,13);
    r1 = _mm_xor_si128(resultat,r1);

    resultat = r1;
    r1 = _mm_srli_epi32(resultat,17);
    r1 = _mm_xor_si128(resultat,r1);

    resultat = r1;
    r1 = _mm_slli_epi32(resultat,5);
    r1 = _mm_xor_si128(resultat,r1);

   *four_states = r1;

}
