/*  ABS FFT Arch (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Arch_arm64_NEON_H
#define PokemonAutomation_Kernels_AbsFFT_Arch_arm64_NEON_H

#include <cstddef>
#include <arm_neon.h>
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
//  Complex multiply of two packed (re/im) pairs:
//      Xr*Wr - Xi*Wi   /   Xi*Wr + Xr*Wi
//  Fused with the vfmsq/vfmaq spellings (as opposed to vmlsq/vmlaq),
//  which expand to the __builtin_aarch64_fma* builtins that both GCC and
//  Clang keep as real FMLA/FMLS instructions; GCC 13 decomposes the
//  vmlaq/vmlsq builtins back into mul + add.
//  The accumulate form is used (first product pre-rounded, second
//  product fused): it takes 4 instructions with no negation, which is
//  also how the compiler naturally contracts the scalar Default path,
//  so NEON stays bit-exact with it. (The x86 fmsub-shaped alternative,
//  pre-rounded Xi*Wi with the fused Xr*Wr, needs a negate and differs
//  from this by a ULP or so in edge cases.)
//  t0/t1 are reused as the output registers; Xr/Xi are still the
//  originals when the FMAs run.
static PA_FORCE_INLINE void cmul_pp(
    vtype& Xr, vtype& Xi,
    vtype Wr, vtype Wi
){
    vtype t0 = vmulq_f32(Xr, Wr);
    vtype t1 = vmulq_f32(Xi, Wr);
    t0 = vfmsq_f32(t0, Xi, Wi);
    t1 = vfmaq_f32(t1, Xr, Wi);
    Xr = t0;
    Xi = t1;
}


static PA_FORCE_INLINE vtype abs(vtype r, vtype i){
    //  sqrt(r*r + i*i), with the mul/add contracted into an FMA
    //  (x86: _mm256_fmadd_ps(r, r, i*i); the two products play swapped
    //  roles here, as in cmul_pp, so it can differ from x86 by a ULP
    //  in edge cases).
    vtype r0 = vfmaq_f32(vmulq_f32(r, r), i, i);
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
