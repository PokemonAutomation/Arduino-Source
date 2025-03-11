/*  ABS FFT Reductions
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_AbsFFT_Reductions_H
#define PokemonAutomation_Kernels_AbsFFT_Reductions_H

#include "Kernels_AbsFFT_Arch_Default.h"
#include "Kernels_AbsFFT_TwiddleTable.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{
template <typename Context>
struct Reductions{

using vtype = typename Context::vtype;


static PA_FORCE_INLINE void fft_real_split_reduce_scalar(const TwiddleTable<Context>& table, int k, float* real, float* upper_complex){
    size_t stride = (size_t)1 << (k - 2);
    for (size_t c = 0; c < stride; c++){
        float r0 = real[c + 0*stride];
        float r1 = real[c + 1*stride];
        float r2 = real[c + 2*stride];
        float r3 = real[c + 3*stride];
        real[c + 0*stride] = r0 + r2;
        real[c + 1*stride] = r1 + r3;

        r0 -= r2;
        r1 -= r3;

        const vcomplex<Context>& vec = table[k].w1[c / Context::VECTOR_LENGTH];
        size_t sindex = c % Context::VECTOR_LENGTH;

        Context_Default::cmul_pp(
            r0, r1,
            vec.real(sindex), vec.imag(sindex)
        );
        upper_complex[2*c + 0] = r0;
        upper_complex[2*c + 1] = r1;
    }
}

static PA_FORCE_INLINE void fft_real_split_reduce(const TwiddleTable<Context>& table, int k, vtype* real, vtype* upper_complex){
    size_t vstride = (size_t)1 << (k - 2 - Context::VECTOR_K);
    vtype* R0 = real;
    vtype* R1 = R0 + vstride;
    vtype* R2 = R1 + vstride;
    vtype* R3 = R2 + vstride;
    const vcomplex<Context>* w = table[k].w1.data();
    size_t lc = vstride;
    do{
        vtype r0 = R0[0];
        vtype r1 = R1[0];
        vtype r2 = R2[0];
        vtype r3 = R3[0];

        R0[0] = Context::vadd(r0, r2);
        R1[0] = Context::vadd(r1, r3);

        r0 = Context::vsub(r0, r2);
        r1 = Context::vsub(r1, r3);
        Context::cmul_pp(r0, r1, w[0].r, w[0].i);

        upper_complex[0] = r0;
        upper_complex[1] = r1;

        R0++;
        R1++;
        R2++;
        R3++;
        upper_complex += 2;
        w++;
    }while (--lc);
}



};
}
}
}
#endif
