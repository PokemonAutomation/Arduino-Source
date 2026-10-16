/*  Scale Invariant Matrix Match
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/CpuId/CpuId.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min4_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min8_x86_AVX2   (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min16_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min4_arm64_NEON(size_t width, size_t height, float const* const* A, float const* const* T);

float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min4_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min8_x86_AVX2   (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min16_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min4_arm64_NEON(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);


float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_min16_x86_AVX512(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (width >= 8 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_min8_x86_AVX2(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_min4_x86_SSE(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_M1){
        return compute_scale_min4_arm64_NEON(width, height, A, T);
    }
#endif
    return compute_scale_Default(width, height, A, T);
}
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_min16_x86_AVX512(width, height, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (width >= 8 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_min8_x86_AVX2(width, height, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_min4_x86_SSE(width, height, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_M1){
        return compute_scale_min4_arm64_NEON(width, height, A, TW, W);
    }
#endif
    return compute_scale_Default(width, height, A, TW, W);
}



float compute_error_Default         (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min4_x86_SSE    (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min8_x86_AVX2   (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min16_x86_AVX512(size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min4_arm64_NEON(size_t width, size_t height, float scale, float const* const* A, float const* const* T);

float compute_error_Default         (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min4_x86_SSE    (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min8_x86_AVX2   (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min16_x86_AVX512(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min4_arm64_NEON(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);

void add_tests_x86_SSE(UnitTestDatabase& database);
void add_tests_x86_AVX2(UnitTestDatabase& database);
void add_tests_x86_AVX512(UnitTestDatabase& database);
void add_tests_arm64_NEON(UnitTestDatabase& database);

float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_error_min16_x86_AVX512(width, height, scale, A, T);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (width >= 8 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_error_min8_x86_AVX2(width, height, scale, A, T);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_error_min4_x86_SSE(width, height, scale, A, T);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_M1){
        return compute_error_min4_arm64_NEON(width, height, scale, A, T);
    }
#endif
    return compute_error_Default(width, height, scale, A, T);
}
float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW,
    float const* const* W
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_error_min16_x86_AVX512(width, height, scale, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (width >= 8 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_error_min8_x86_AVX2(width, height, scale, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_error_min4_x86_SSE(width, height, scale, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (width >= 4 && CPU_CAPABILITY_CURRENT.OK_M1){
        return compute_error_min4_arm64_NEON(width, height, scale, A, TW, W);
    }
#endif
    return compute_error_Default(width, height, scale, A, TW, W);
}


// 
//  Register the tests for the ISAs this binary can run. Each ISA checks its
//  results against the Default path, which is the source of truth, so the
//  Default path itself gets no tests.
// 
void add_tests(UnitTestDatabase& database){
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        add_tests_x86_SSE(database);
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
