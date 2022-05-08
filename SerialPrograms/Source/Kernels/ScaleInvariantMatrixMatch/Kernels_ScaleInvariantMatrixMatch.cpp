/*  Scale Invariant Matrix Match
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min16_x86_SSE   (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min32_x86_FMA3  (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_min64_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* T);

float compute_scale_Default         (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min16_x86_SSE   (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min32_x86_FMA3  (size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);
float compute_scale_min64_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* TW, float const* const* W);


float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
#ifdef PA_AutoDispatch_17_Skylake
    if (width >= 64 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_min64_x86_AVX512(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (width >= 32 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_min32_x86_FMA3(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_min16_x86_SSE(width, height, A, T);
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
#ifdef PA_AutoDispatch_17_Skylake
    if (width >= 64 && CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_min64_x86_AVX512(width, height, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (width >= 32 && CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_min32_x86_FMA3(width, height, A, TW, W);
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (width >= 16 && CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_min16_x86_SSE(width, height, A, TW, W);
    }
#endif
    return compute_scale_Default(width, height, A, TW, W);
}






}
}
}
