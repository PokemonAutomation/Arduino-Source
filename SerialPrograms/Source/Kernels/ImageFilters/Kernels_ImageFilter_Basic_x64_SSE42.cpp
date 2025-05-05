/*  Image Filters Basic (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <stdint.h>
#include <cstddef>
#include <nmmintrin.h>
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


#if 0

struct PartialWordMask{
    size_t left;
    PartialWordAccess_x64_SSE41 loader;

    PA_FORCE_INLINE PartialWordMask(size_t p_left)
        : left(p_left)
        , loader(left * sizeof(uint32_t))
    {}
};

#endif
















}
}
#endif
