/*  Scale Invariant Matrix Match
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_H
#define PokemonAutomation_Kernels_ScaleInvariantMatrixMatch_H

namespace PokemonAutomation{
namespace Kernels{
namespace ScaleInvariantMatrixMatch{


//  Compute s that minimizes: |s A - T|^2
//      All pointers must have the same alignment.
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* T
);


//  Compute s that minimizes: (|s A - T| * W)^2
//      All pointers must have the same alignment.
float compute_scale(
    size_t width, size_t height,
    float const* const* A,
    float const* const* TW, //  Precomputed T * W
    float const* const* W
);



//  Compute: |s A - T|^2
//      All pointers must have the same alignment.
float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* T
);


//  Compute: |s A w - T|^2
//      All pointers must have the same alignment.
float compute_error(
    size_t width, size_t height,
    float scale,
    float const* const* A,
    float const* const* TW, //  Precomputed T * W
    float const* const* W
);





}
}
}
#endif
