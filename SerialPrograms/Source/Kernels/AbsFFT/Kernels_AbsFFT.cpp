/*  ABS FFT
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stddef.h>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_AbsFFT.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



void fft_abs_Default(int k, float* abs, float* real);
void fft_abs_x86_SSE41(int k, float* abs, float* real);
void fft_abs_x86_AVX2(int k, float* abs, float* real);


void fft_abs(int k, float* abs, float* real){
    if (k <= 0){
        throw "FFT length must be at least 2^1.";
    }
    if ((size_t)abs & 63){
        throw "abs must be aligned to 64 bytes.";
    }
    if ((size_t)real & 63){
        throw "real must be aligned to 64 bytes.";
    }

#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        fft_abs_x86_AVX2(k, abs, real);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        fft_abs_x86_SSE41(k, abs, real);
        return;
    }
#endif
    fft_abs_Default(k, abs, real);
}



}
}
}
