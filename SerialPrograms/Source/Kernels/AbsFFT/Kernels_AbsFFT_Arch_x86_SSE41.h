/*  ABS FFT Arch (SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_x86_SSE41_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_x86_SSE41_H

#include <smmintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
struct Context_x86_SSE41{


using vtype = __m128;

static const int VECTOR_K = 2;
static const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

static const int BASE_COMPLEX_TRANSFORM_K = 4;
static const size_t MIN_TABLE_WIDTH = 1;


static PA_FORCE_INLINE vtype vset1(float x){
    return _mm_set1_ps(x);
}
static PA_FORCE_INLINE vtype vneg(vtype x){
    return _mm_xor_ps(x, _mm_set1_ps(-0.0));
}
static PA_FORCE_INLINE vtype vadd(vtype x, vtype y){
    return _mm_add_ps(x, y);
}
static PA_FORCE_INLINE vtype vsub(vtype x, vtype y){
    return _mm_sub_ps(x, y);
}
static PA_FORCE_INLINE vtype vmul(vtype x, vtype y){
    return _mm_mul_ps(x, y);
}
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = _mm_mul_ps(Xi, Wi);
    vtype t1 = _mm_mul_ps(Xr, Wi);
    Xr = _mm_mul_ps(Xr, Wr);
    Xr = _mm_sub_ps(Xr, t0);
    Xi = _mm_mul_ps(Xi, Wr);
    Xi = _mm_add_ps(Xi, t1);
}


static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    vtype r0 = _mm_add_ps(_mm_mul_ps(r, r), _mm_mul_ps(i, i));
    return _mm_sqrt_ps(r0);
}
static PA_FORCE_INLINE void swap_odd(vtype& L, vtype& H){
    vtype r0 = _mm_shuffle_ps(L, L, 108);
    vtype r1 = _mm_shuffle_ps(H, H, 108);
    L = _mm_blend_ps(L, r1, 10);
    H = _mm_blend_ps(H, r0, 10);
}


static PA_FORCE_INLINE void interleave_v0(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = _mm_unpacklo_ps(lo, hi);
    out1 = _mm_unpackhi_ps(lo, hi);
}
static PA_FORCE_INLINE void interleave_v1(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = _mm_shuffle_ps(lo, hi, 68);
    out1 = _mm_shuffle_ps(lo, hi, 238);
}



};
}
}
}
#endif
