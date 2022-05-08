/*  Scale Invariant Matrix Match Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_Routines_H
#define PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_Routines_H

#include <stddef.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Exceptions.h"

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{



template <typename SumATA2>
PA_FORCE_INLINE float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
){
    constexpr size_t ALIGNMENT = alignof(typename SumATA2::vtype);
    SumATA2 sum;
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = T[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A and T must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT);
    }
    return sum.scale();
}
template <typename SumATA2>
PA_FORCE_INLINE float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,
    float const* const* W2
){
    constexpr size_t ALIGNMENT = alignof(typename SumATA2::vtype);
    SumATA2 sum;
    for (size_t r = 0; r < height; r++){
        const float* ptrA = A[r];
        const float* ptrT = TW2[r];
        const float* ptrW = W2[r];
        if ((size_t)ptrA % ALIGNMENT != (size_t)ptrT % ALIGNMENT || (size_t)ptrA % ALIGNMENT != (size_t)ptrW % ALIGNMENT){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "A, TW2, and W2 must have the same alignment.");
        }
        sum.accumulate(width, ptrA, ptrT, ptrW);
    }
    return sum.scale();
}



}
}
}
#endif
