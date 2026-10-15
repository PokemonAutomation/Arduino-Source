/*  ABS FFT Base Transform (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_BaseTransform_arm64_NEON_H
#define PokemonAutomation_Kernels_AbsFFT_BaseTransform_arm64_NEON_H

#include "Kernels_AbsFFT_Arch_arm64_NEON.h"
#include "Kernels_AbsFFT_Butterflies.h"
#include "Kernels_AbsFFT_ComplexVector.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{


PA_FORCE_INLINE void vtranspose(float32x4_t& r0, float32x4_t& r1, float32x4_t& r2, float32x4_t& r3){
    float32x4_t a0 = vzip1q_f32(r0, r1);
    float32x4_t a1 = vzip2q_f32(r0, r1);
    float32x4_t a2 = vzip1q_f32(r2, r3);
    float32x4_t a3 = vzip2q_f32(r2, r3);
    r0 = vcombine_f32(vget_low_f32(a0), vget_low_f32(a2));
    r1 = vcombine_f32(vget_high_f32(a0), vget_high_f32(a2));
    r2 = vcombine_f32(vget_low_f32(a1), vget_low_f32(a3));
    r3 = vcombine_f32(vget_high_f32(a1), vget_high_f32(a3));
}


template <>
void base_transform<Context_arm64_NEON>(const TwiddleTable<Context_arm64_NEON>& table, Context_arm64_NEON::vtype* T){
    float32x4_t r0, r1, r2, r3;
    float32x4_t i0, i1, i2, i3;

    r0 = T[0];
    i0 = T[1];
    r1 = T[2];
    i1 = T[3];
    r2 = T[4];
    r3 = T[6];
    i2 = T[5];
    i3 = T[7];

    const vcomplex<Context_arm64_NEON>* w1 = table[3].w1.data();
    const vcomplex<Context_arm64_NEON>* w2 = table[4].w1.data();
    const vcomplex<Context_arm64_NEON>* w3 = table[4].w3.data();
    Butterflies<Context_arm64_NEON>::butterfly4(
        r0, i0,
        r1, i1, w1[0].r, w1[0].i,
        r2, i2, w2[0].r, w2[0].i,
        r3, i3, w3[0].r, w3[0].i
    );

    vtranspose(r0, r1, r2, r3);
    vtranspose(i0, i1, i2, i3);

    Butterflies<Context_arm64_NEON>::butterfly4(
        r0, i0,
        r1, i1,
        r2, i2,
        r3, i3
    );

    vtranspose(r0, r1, r2, r3);
    T[0] = r0;
    T[2] = r1;
    T[4] = r2;
    T[6] = r3;
    vtranspose(i0, i1, i2, i3);
    T[1] = i0;
    T[3] = i1;
    T[5] = i2;
    T[7] = i3;
}



}
}
}
#endif
