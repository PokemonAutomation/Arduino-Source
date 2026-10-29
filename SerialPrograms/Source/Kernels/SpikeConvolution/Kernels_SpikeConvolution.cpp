/*  Spike Convolution
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels_SpikeConvolution.h"

namespace PokemonAutomation{
namespace Kernels{
namespace SpikeConvolution{


void compute_spike_kernel_Default   (float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);
void compute_spike_kernel_x86_SSE41 (float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);
void compute_spike_kernel_x86_AVX2  (float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);
void compute_spike_kernel_x86_AVX512(float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);
void compute_spike_kernel_arm64_NEON(float* out, const float* in, size_t lengthI, const float* kernel, size_t lengthK);

void add_tests_x86_SSE41(UnitTestDatabase& database);
void add_tests_x86_AVX2 (UnitTestDatabase& database);
void add_tests_x86_AVX512(UnitTestDatabase& database);
void add_tests_arm64_NEON(UnitTestDatabase& database);

void add_tests_x86_SSE41(UnitTestDatabase& database);
void add_tests_x86_AVX2 (UnitTestDatabase& database);
void add_tests_x86_AVX512(UnitTestDatabase& database);


void compute_spike_kernel(
    float* out, const float* in, size_t lengthI,
    const float* kernel, size_t lengthK
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_spike_kernel_x86_AVX512(out, in, lengthI, kernel, lengthK);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_spike_kernel_x86_AVX2(out, in, lengthI, kernel, lengthK);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_spike_kernel_x86_SSE41(out, in, lengthI, kernel, lengthK);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return compute_spike_kernel_arm64_NEON(out, in, lengthI, kernel, lengthK);
    }
#endif
    return compute_spike_kernel_Default(out, in, lengthI, kernel, lengthK);
}


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
