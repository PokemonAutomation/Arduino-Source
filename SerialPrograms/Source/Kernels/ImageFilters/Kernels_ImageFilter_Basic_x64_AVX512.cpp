/*  Image Filters Basic (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{



#if 0

namespace{

struct PartialWordMask{
    __mmask16 m;

    PA_FORCE_INLINE PartialWordMask(size_t left)
        : m(((__mmask16)1 << left) - 1)
    {}
};

}

#endif





















}
}
#endif
