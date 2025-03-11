/*  Kernels (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_x64_AVX512_H
#define PokemonAutomation_Kernels_x64_AVX512_H

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"

#include <iostream>

namespace PokemonAutomation{
namespace Kernels{


inline void print_u8(__m512i x){
    for (int i = 0; i < 64; i++){
        std::cout << (int)((const unsigned char*)&x)[i] << " ";
    }
    std::cout << std::endl;
}
inline void print_u16(const __m512i& x){
    union{
        __m512i v;
        uint16_t s[32];
    };
    v = x;
    for (int i = 0; i < 32; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline void print_u32(const __m512i& x){
    union{
        __m512i v;
        uint32_t s[16];
    };
    v = x;
    for (int i = 0; i < 16; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}
inline void print_u64(const __m512i& x){
    union{
        __m512i v;
        uint64_t s[8];
    };
    v = x;
    for (int i = 0; i < 8; i++){
        std::cout << s[i] << " ";
    }
    std::cout << std::endl;
}



#define _mm512_setr_epi16(  \
    e00, e01, e02, e03, e04, e05, e06, e07, \
    e08, e09, e10, e11, e12, e13, e14, e15, \
    e16, e17, e18, e19, e20, e21, e22, e23, \
    e24, e25, e26, e27, e28, e29, e30, e31  \
)   \
    _mm512_setr_epi32(  \
        (uint16_t)(e00) | ((uint32_t)(e01) << 16), (uint16_t)(e02) | ((uint32_t)(e03) << 16), (uint16_t)(e04) | ((uint32_t)(e05) << 16), (uint16_t)(e06) | ((uint32_t)(e07) << 16),  \
        (uint16_t)(e08) | ((uint32_t)(e09) << 16), (uint16_t)(e10) | ((uint32_t)(e11) << 16), (uint16_t)(e12) | ((uint32_t)(e13) << 16), (uint16_t)(e14) | ((uint32_t)(e15) << 16),  \
        (uint16_t)(e16) | ((uint32_t)(e17) << 16), (uint16_t)(e18) | ((uint32_t)(e19) << 16), (uint16_t)(e20) | ((uint32_t)(e21) << 16), (uint16_t)(e22) | ((uint32_t)(e23) << 16),  \
        (uint16_t)(e24) | ((uint32_t)(e25) << 16), (uint16_t)(e26) | ((uint32_t)(e27) << 16), (uint16_t)(e28) | ((uint32_t)(e29) << 16), (uint16_t)(e30) | ((uint32_t)(e31) << 16)   \
    )
#define _mm512_setr_epi8(   \
    e00, e01, e02, e03, e04, e05, e06, e07, \
    e08, e09, e10, e11, e12, e13, e14, e15, \
    e16, e17, e18, e19, e20, e21, e22, e23, \
    e24, e25, e26, e27, e28, e29, e30, e31, \
    e32, e33, e34, e35, e36, e37, e38, e39, \
    e40, e41, e42, e43, e44, e45, e46, e47, \
    e48, e49, e50, e51, e52, e53, e54, e55, \
    e56, e57, e58, e59, e60, e61, e62, e63  \
)   \
    _mm512_setr_epi16(  \
        (uint8_t)(e00) | ((uint8_t)(e01) << 8), (uint8_t)(e02) | ((uint8_t)(e03) << 8), (uint8_t)(e04) | ((uint8_t)(e05) << 8), (uint8_t)(e06) | ((uint8_t)(e07) << 8), \
        (uint8_t)(e08) | ((uint8_t)(e09) << 8), (uint8_t)(e10) | ((uint8_t)(e11) << 8), (uint8_t)(e12) | ((uint8_t)(e13) << 8), (uint8_t)(e14) | ((uint8_t)(e15) << 8), \
        (uint8_t)(e16) | ((uint8_t)(e17) << 8), (uint8_t)(e18) | ((uint8_t)(e19) << 8), (uint8_t)(e20) | ((uint8_t)(e21) << 8), (uint8_t)(e22) | ((uint8_t)(e23) << 8), \
        (uint8_t)(e24) | ((uint8_t)(e25) << 8), (uint8_t)(e26) | ((uint8_t)(e27) << 8), (uint8_t)(e28) | ((uint8_t)(e29) << 8), (uint8_t)(e30) | ((uint8_t)(e31) << 8), \
        (uint8_t)(e32) | ((uint8_t)(e33) << 8), (uint8_t)(e34) | ((uint8_t)(e35) << 8), (uint8_t)(e36) | ((uint8_t)(e37) << 8), (uint8_t)(e38) | ((uint8_t)(e39) << 8), \
        (uint8_t)(e40) | ((uint8_t)(e41) << 8), (uint8_t)(e42) | ((uint8_t)(e43) << 8), (uint8_t)(e44) | ((uint8_t)(e45) << 8), (uint8_t)(e46) | ((uint8_t)(e47) << 8), \
        (uint8_t)(e48) | ((uint8_t)(e49) << 8), (uint8_t)(e50) | ((uint8_t)(e51) << 8), (uint8_t)(e52) | ((uint8_t)(e53) << 8), (uint8_t)(e54) | ((uint8_t)(e55) << 8), \
        (uint8_t)(e56) | ((uint8_t)(e57) << 8), (uint8_t)(e58) | ((uint8_t)(e59) << 8), (uint8_t)(e60) | ((uint8_t)(e61) << 8), (uint8_t)(e62) | ((uint8_t)(e63) << 8)  \
    )



PA_FORCE_INLINE void transpose_i64_8x8_AVX512(
    __m512i& r0, __m512i& r1, __m512i& r2, __m512i& r3,
    __m512i& r4, __m512i& r5, __m512i& r6, __m512i& r7
){
    __m512i a0, a1;

    a0 = _mm512_unpackhi_epi64(r0, r1);
    r1 = _mm512_unpacklo_epi64(r0, r1);
    a1 = _mm512_unpackhi_epi64(r2, r3);
    r2 = _mm512_unpacklo_epi64(r2, r3);
    r0 = _mm512_unpackhi_epi64(r4, r5);
    r4 = _mm512_unpacklo_epi64(r4, r5);
    r5 = _mm512_unpacklo_epi64(r6, r7);
    r7 = _mm512_unpackhi_epi64(r6, r7);

    r3 = _mm512_shuffle_i64x2(r1, r2, 221);
    r1 = _mm512_shuffle_i64x2(r1, r2, 136);
    r2 = _mm512_shuffle_i64x2(a0, a1, 136);
    a1 = _mm512_shuffle_i64x2(a0, a1, 221);
    r6 = _mm512_shuffle_i64x2(r4, r5, 221);
    r4 = _mm512_shuffle_i64x2(r4, r5, 136);
    r5 = _mm512_shuffle_i64x2(r0, r7, 136);
    r7 = _mm512_shuffle_i64x2(r0, r7, 221);

    r0 = _mm512_shuffle_i64x2(r1, r4, 136);
    r4 = _mm512_shuffle_i64x2(r1, r4, 221);
    r1 = _mm512_shuffle_i64x2(r2, r5, 136);
    r5 = _mm512_shuffle_i64x2(r2, r5, 221);
    r2 = _mm512_shuffle_i64x2(r3, r6, 136);
    r6 = _mm512_shuffle_i64x2(r3, r6, 221);
    r3 = _mm512_shuffle_i64x2(a1, r7, 136);
    r7 = _mm512_shuffle_i64x2(a1, r7, 221);
}


}
}
#endif
