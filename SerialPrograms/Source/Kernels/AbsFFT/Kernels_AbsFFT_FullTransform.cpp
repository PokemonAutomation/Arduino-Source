/*  ABS FFT Full Transform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include <cmath>
#include "Kernels_AbsFFT_BitReverse.h"
#include "Kernels_AbsFFT_ComplexToAbs.h"
#include "Kernels_AbsFFT_Reductions.h"
#include "Kernels_AbsFFT_ComplexScalar.h"
#include "Kernels_AbsFFT_ComplexVector.h"
#include "Kernels_AbsFFT_FullTransform.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



void fft_abs_k1(float abs[1], float real[2]){
    abs[0] = std::abs(real[0] + real[1]);
}
void fft_abs_k2(float abs[2], float real[4]){
    float t0, t1, t2, t3;
    t0 = real[0] + real[2];
    t2 = real[0] - real[2];
    t1 = real[1] + real[3];
    t3 = real[1] - real[3];
    t0 += t1;
    abs[0] = std::abs(t0);
    abs[1] = std::sqrt(t2*t2 + t3*t3);
}
void fft_abs_k3(const TwiddleTable& table, float abs[4], float real[8]){
    const int k = 3;
    const size_t block = (size_t)1 << (k - 2);

    //  Initial split-radix reduction.
    fft_real_split_reduce(table, k, real, abs);

    //  Transform complex upper-half.
    fft_complex_tk_scalar(table, k - 2, abs);

    //  Compute abs and reverse order of odd terms.
    MiscRoutines<Intrinsics_Default>::complex_to_abs(real + 3*block, abs, block);

    //  Recurse into lower-half.
    fft_abs_k2(real + 2*block, real);

    //  Fix ordering.
    real += 2*block;

    abs[0] = real[0];
    abs[1] = real[2];
    abs[2] = real[1];
    abs[3] = real[3];
}
void fft_abs_scalar(const TwiddleTable& table, int k, float* abs, float* real){
    if (k == 1){
        fft_abs_k1(abs, real);
        return;
    }
    if (k == 2){
        fft_abs_k2(abs, real);
        return;
    }
    if (k == 3){
        fft_abs_k3(table, abs, real);
        return;
    }

    size_t block = (size_t)1 << (k - 2);

    //  Initial split-radix reduction.
    fft_real_split_reduce(table, k, real, abs);

    //  Transform complex upper-half.
    fft_complex_tk_scalar(table, k - 2, abs);

    //  Compute abs and reverse order of odd terms.
    MiscRoutines<Intrinsics_Default>::complex_to_abs_swap_odd(real + 3*block, abs, block);

    //  Recurse into lower-half.
    fft_abs_scalar(table, k - 1, real + 2*block, real);

    //  Fix ordering.
    real += 2*block;
    bitreverse_f32v1_ip(k - 2, real + block, abs);
    interleave_f32(k - 1, abs, real);
}


void fft_abs(const TwiddleTable& table, int k, float* abs, float* real){
    if (k - 2 < Intrinsics::BASE_COMPLEX_TRANSFORM_K){
        fft_abs_scalar(table, k, abs, real);
        return;
    }

    size_t block = (size_t)1 << (k - 2);

    //  Initial split-radix reduction.
    fft_real_split_reduce(table, k, (vtype*)real, (vtype*)abs);

    //  Transform complex upper-half.
    fft_complex_tk(table, k - 2, (vtype*)abs);

    //  Compute abs and reverse order of odd terms.
    MiscRoutines<Intrinsics>::complex_to_abs_swap_odd(
        (vtype*)(real + 3*block),
        (vtype*)(abs),
        block >> VECTOR_K
    );

    //  Recurse into lower-half.
    fft_abs(table, k - 1, real + 2*block, real);

    //  Fix ordering.
    real += 2*block;
    bitreverse_f32v1_ip(k - 2, real + block, abs);
    interleave_f32(k - 1, abs, real);
}







}
}
}
