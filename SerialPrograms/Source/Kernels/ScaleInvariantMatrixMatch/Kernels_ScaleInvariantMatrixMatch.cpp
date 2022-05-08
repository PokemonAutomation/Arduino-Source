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


float compute_scale_u2_Default    (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_u8_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_u16_x86_FMA3  (size_t width, size_t height, float const* const* A, float const* const* T);
float compute_scale_u32_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* T);

float compute_scale_u2_Default    (size_t width, size_t height, float const* const* A, float const* const* TW2, float const* const* W2);
float compute_scale_u8_x86_SSE    (size_t width, size_t height, float const* const* A, float const* const* TW2, float const* const* W2);
float compute_scale_u16_x86_FMA3  (size_t width, size_t height, float const* const* A, float const* const* TW2, float const* const* W2);
float compute_scale_u32_x86_AVX512(size_t width, size_t height, float const* const* A, float const* const* TW2, float const* const* W2);


float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    if (width % 64){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width must be a multiple of 64.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_u32_x86_AVX512(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_u16_x86_FMA3(width, height, A, T);
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_u8_x86_SSE(width, height, A, T);
    }
#endif
    return compute_scale_u2_Default(width, height, A, T);
}
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    if (width % 64){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width must be a multiple of 64.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return compute_scale_u32_x86_AVX512(width, height, A, TW2, W2);
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return compute_scale_u16_x86_FMA3(width, height, A, TW2, W2);
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return compute_scale_u8_x86_SSE(width, height, A, TW2, W2);
    }
#endif
    return compute_scale_u2_Default(width, height, A, TW2, W2);
}






}
}
}
