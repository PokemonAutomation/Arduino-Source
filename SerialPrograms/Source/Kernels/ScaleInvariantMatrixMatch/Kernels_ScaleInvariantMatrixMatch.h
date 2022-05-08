/*  Scale Invariant Matrix Match
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_H
#define PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_H

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


//  Compute s that minimizes: |s A - T|^2
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
);

//  Compute s that minimizes: (|s A - T| * W)^2
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW2,    //  Precomputed T * W^2
    float const* const* W2      //  Precomputed W^2
);



}
}
}
#endif
