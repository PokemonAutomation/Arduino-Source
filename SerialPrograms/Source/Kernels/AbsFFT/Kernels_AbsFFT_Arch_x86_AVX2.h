/*  ABS FFT Arch (AVX2)
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
struct Context_x86_AVX2{


using vtype = __m256;
static const int VECTOR_K = 3;
static const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

static const int BASE_COMPLEX_TRANSFORM_K = 6;
static const size_t MIN_TABLE_WIDTH = 4;


static PA_FORCE_INLINE vtype vset1(float x){
    return _mm256_set1_ps(x);
}
static PA_FORCE_INLINE vtype vneg(vtype x){
    return _mm256_xor_ps(x, _mm256_set1_ps(-0.0));
}
static PA_FORCE_INLINE vtype vadd(vtype x, vtype y){
    return _mm256_add_ps(x, y);
}
static PA_FORCE_INLINE vtype vsub(vtype x, vtype y){
    return _mm256_sub_ps(x, y);
}
static PA_FORCE_INLINE vtype vmul(vtype x, vtype y){
    return _mm256_mul_ps(x, y);
}
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = _mm256_mul_ps(Xi, Wi);
    vtype t1 = _mm256_mul_ps(Xr, Wi);
    Xr = _mm256_fmsub_ps(Xr, Wr, t0);
    Xi = _mm256_fmadd_ps(Xi, Wr, t1);
}


static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    vtype r0 = _mm256_fmadd_ps(r, r, _mm256_mul_ps(i, i));
    return _mm256_sqrt_ps(r0);
}
static PA_FORCE_INLINE void swap_odd(vtype& L, vtype& H){
    vtype r0 = _mm256_permutevar8x32_ps(L, _mm256_setr_epi32(0, 7, 2, 5, 4, 3, 6, 1));
    vtype r1 = _mm256_permutevar8x32_ps(H, _mm256_setr_epi32(0, 7, 2, 5, 4, 3, 6, 1));
    L = _mm256_blend_ps(L, r1, 170);
    H = _mm256_blend_ps(H, r0, 170);
}


static PA_FORCE_INLINE void interleave_v0(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    __m256 a0 = _mm256_permute2f128_ps(lo, hi, 32);
    __m256 a1 = _mm256_permute2f128_ps(lo, hi, 49);
    out0 = _mm256_permutevar8x32_ps(a0, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
    out1 = _mm256_permutevar8x32_ps(a1, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
}
static PA_FORCE_INLINE void interleave_v1(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    __m256 a0 = _mm256_permute2f128_ps(lo, hi, 32);
    __m256 a1 = _mm256_permute2f128_ps(lo, hi, 49);
    out0 = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(a0), 216));
    out1 = _mm256_castpd_ps(_mm256_permute4x64_pd(_mm256_castps_pd(a1), 216));
}


};
}
}
}
#endif
