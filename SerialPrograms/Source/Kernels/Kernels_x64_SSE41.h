/*  Kernels (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_x64_SSE41_H
#define PokemonAutomation_Kernels_x64_SSE41_H

#include <stdint.h>
#include <smmintrin.h>
#include "Common/Compiler.h"

#include <iostream>

namespace PokemonAutomation{
namespace Kernels{


inline static void print(const __m128& x){
    union{
        __m128 v;
        float s[4];
    };
    v = x;
    for (int i = 0; i < 4; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u8(__m128i x){
    for (int i = 0; i < 16; i++){
        std::cout << (int)((const unsigned char*)&x)[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u16(const __m128i& x){
    union{
        __m128i v;
        uint16_t s[8];
    };
    v = x;
    for (int i = 0; i < 8; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u32(const __m128i& x){
    union{
        __m128i v;
        uint32_t s[4];
    };
    v = x;
    for (int i = 0; i < 4; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline static void print_u64(const __m128i& x){
    union{
        __m128i v;
        uint64_t s[2];
    };
    v = x;
    for (int i = 0; i < 2; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}




PA_FORCE_INLINE float reduce32_x64_SSE(__m128 x){
    x = _mm_add_ps(x, _mm_shuffle_ps(x, x, 78));
    x = _mm_add_ps(x, _mm_shuffle_ps(x, x, 177));
    return _mm_cvtss_f32(x);
}
PA_FORCE_INLINE uint64_t reduce32_x64_SSE41(__m128i x){
    uint64_t ret = _mm_cvtsi128_si32(x);
    ret += _mm_extract_epi32(x, 1);
    ret += _mm_extract_epi32(x, 2);
    ret += _mm_extract_epi32(x, 3);
    return ret;
}

PA_FORCE_INLINE void transpose_i64_2x2_SSE2(__m128i& r0, __m128i& r1){
    __m128i a0 = r0;
    r0 = _mm_unpacklo_epi64(r0, r1);
    r1 = _mm_unpackhi_epi64(a0, r1);
}




}
}
#endif
