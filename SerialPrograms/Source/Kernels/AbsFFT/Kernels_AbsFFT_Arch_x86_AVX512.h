/*  ABS FFT Arch (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_x86_AVX2_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_x86_AVX2_H

#include <immintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
struct Context_x86_AVX512{


using vtype = __m512;
static const int VECTOR_K = 4;
static const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

static const int BASE_COMPLEX_TRANSFORM_K = 8;
static const size_t MIN_TABLE_WIDTH = 8;


static PA_FORCE_INLINE vtype vset1(float x){
    return _mm512_set1_ps(x);
}
static PA_FORCE_INLINE vtype vneg(vtype x){
    return _mm512_xor_ps(x, _mm512_set1_ps(-0.0));
}
static PA_FORCE_INLINE vtype vadd(vtype x, vtype y){
    return _mm512_add_ps(x, y);
}
static PA_FORCE_INLINE vtype vsub(vtype x, vtype y){
    return _mm512_sub_ps(x, y);
}
static PA_FORCE_INLINE vtype vmul(vtype x, vtype y){
    return _mm512_mul_ps(x, y);
}
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = _mm512_mul_ps(Xi, Wi);
    vtype t1 = _mm512_mul_ps(Xr, Wi);
    Xr = _mm512_fmsub_ps(Xr, Wr, t0);
    Xi = _mm512_fmadd_ps(Xi, Wr, t1);
}


static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    vtype r0 = _mm512_fmadd_ps(r, r, _mm512_mul_ps(i, i));
    return _mm512_sqrt_ps(r0);
}
static PA_FORCE_INLINE void swap_odd(vtype& L, vtype& H){
    vtype r0 = _mm512_permutex2var_ps(L, _mm512_setr_epi32( 0, 31,  2, 29,  4, 27,  6, 25,  8, 23, 10, 21, 12, 19, 14, 17), H);
    vtype r1 = _mm512_permutex2var_ps(L, _mm512_setr_epi32(16, 15, 18, 13, 20, 11, 22,  9, 24,  7, 26,  5, 28,  3, 30,  1), H);
    L = r0;
    H = r1;
}


static PA_FORCE_INLINE void interleave_v0(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = _mm512_permutex2var_ps(lo, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), hi);
    out1 = _mm512_permutex2var_ps(lo, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), hi);
}
static PA_FORCE_INLINE void interleave_v1(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = _mm512_permutex2var_ps(lo, _mm512_setr_epi32( 0,  1, 16, 17,  2,  3, 18, 19,  4,  5, 20, 21,  6,  7, 22, 23), hi);
    out1 = _mm512_permutex2var_ps(lo, _mm512_setr_epi32( 8,  9, 24, 25, 10, 11, 26, 27, 12, 13, 28, 29, 14, 15, 30, 31), hi);
}


};
}
}
}
#endif
