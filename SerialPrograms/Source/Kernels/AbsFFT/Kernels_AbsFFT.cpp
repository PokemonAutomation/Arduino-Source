/*  ABS FFT
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stddef.h>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_AbsFFT.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AbsFFT{



void fft_abs_Default(int k, float* abs, float* real, const float* weights);
void fft_abs_x86_SSE41(int k, float* abs, float* real, const float* weights);
void fft_abs_x86_AVX2(int k, float* abs, float* real, const float* weights);
void fft_abs_x86_AVX512(int k, float* abs, float* real, const float* weights);
void fft_abs_arm64_NEON(int k, float* abs, float* real, const float* weights);

void add_tests_x86_SSE41(UnitTestDatabase& database);
void add_tests_x86_AVX2(UnitTestDatabase& database);
void add_tests_x86_AVX512(UnitTestDatabase& database);
void add_tests_arm64_NEON(UnitTestDatabase& database);


void fft_abs(int k, float* abs, float* real, const float* weights){
    if (k <= 0){
        throw "FFT length must be at least 2^1.";
    }
    if ((size_t)abs & 63){
        throw "abs must be aligned to 64 bytes.";
    }
    if ((size_t)real & 63){
        throw "real must be aligned to 64 bytes.";
    }

#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        fft_abs_x86_AVX512(k, abs, real, weights);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        fft_abs_x86_AVX2(k, abs, real, weights);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        fft_abs_x86_SSE41(k, abs, real, weights);
        return;
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        fft_abs_arm64_NEON(k, abs, real, weights);
        return;
    }
#endif
    fft_abs_Default(k, abs, real, weights);
}



//
//  Register the tests for the ISAs this binary can run. Each ISA checks its
//  results against the Default path, which is the source of truth, so the
//  Default path itself gets no tests.
//
void add_tests(UnitTestDatabase& database){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x86_SSE41(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        add_tests_x86_AVX2(database);
    }
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        add_tests_x86_AVX512(database);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        add_tests_arm64_NEON(database);
    }
#endif
}



}
}
}
