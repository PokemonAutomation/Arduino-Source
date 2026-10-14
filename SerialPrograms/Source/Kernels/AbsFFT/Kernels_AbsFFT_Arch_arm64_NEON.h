/*  ABS FFT Arch (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_arm64_NEON_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_arm64_NEON_H

#include <arm_neon.h>
#include <cstddef>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
struct Context_arm64_NEON{


using vtype = float32x4_t;

static const int VECTOR_K = 2;
static const size_t VECTOR_LENGTH = (size_t)1 << VECTOR_K;

static const int BASE_COMPLEX_TRANSFORM_K = 4;
static const size_t MIN_TABLE_WIDTH = 1;


static PA_FORCE_INLINE vtype vset1(float x){
    return vdupq_n_f32(x);
}
static PA_FORCE_INLINE vtype vneg(vtype x){
    return vnegq_f32(x);
}
static PA_FORCE_INLINE vtype vadd(vtype x, vtype y){
    return vaddq_f32(x, y);
}
static PA_FORCE_INLINE vtype vsub(vtype x, vtype y){
    return vsubq_f32(x, y);
}
static PA_FORCE_INLINE vtype vmul(vtype x, vtype y){
    return vmulq_f32(x, y);
}
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = vmulq_f32(Xi, Wi);
    vtype t1 = vmulq_f32(Xr, Wi);
    Xr = vsubq_f32(vmulq_f32(Xr, Wr), t0);
    Xi = vaddq_f32(vmulq_f32(Xi, Wr), t1);
}


static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    vtype r0 = vaddq_f32(vmulq_f32(r, r), vmulq_f32(i, i));
    return vsqrtq_f32(r0);
}
static PA_FORCE_INLINE void swap_odd(vtype& L, vtype& H){
    //  L' = (L0, H3, L2, H1), H' = (H0, L3, H2, L1)
    float l1 = vgetq_lane_f32(L, 1);
    float l3 = vgetq_lane_f32(L, 3);
    float h1 = vgetq_lane_f32(H, 1);
    float h3 = vgetq_lane_f32(H, 3);
    L = vsetq_lane_f32(h1, vsetq_lane_f32(h3, L, 1), 3);
    H = vsetq_lane_f32(l1, vsetq_lane_f32(l3, H, 1), 3);
}


static PA_FORCE_INLINE void interleave_v0(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = vzip1q_f32(lo, hi);
    out1 = vzip2q_f32(lo, hi);
}
static PA_FORCE_INLINE void interleave_v1(
    vtype& out0, vtype& out1,
    vtype lo, vtype hi
){
    out0 = vcombine_f32(vget_low_f32(lo), vget_low_f32(hi));
    out1 = vcombine_f32(vget_high_f32(lo), vget_high_f32(hi));
}



};
}
}
}
#endif
