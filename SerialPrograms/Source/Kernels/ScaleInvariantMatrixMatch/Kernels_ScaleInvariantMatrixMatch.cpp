/*  Scale Invariant Matrix Match
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min4_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min8_x86_AVX2   (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min16_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* T);

float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min4_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min8_x86_AVX2   (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min16_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);


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
    return compute_scale_Default(width, height, A, TW, W);
}



float compute_error_Default         (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min4_x86_SSE    (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min8_x86_AVX2   (size_t width, size_t height, float scale, float const* const* A, float const* const* T);
float compute_error_min16_x86_AVX512(size_t width, size_t height, float scale, float const* const* A, float const* const* T);

float compute_error_Default         (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min4_x86_SSE    (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min8_x86_AVX2   (size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);
float compute_error_min16_x86_AVX512(size_t width, size_t height, float scale, float const* const* A, float const* const* TW, float const* const* W);

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
    return compute_error_Default(width, height, scale, A, TW, W);
}








}
}
}
