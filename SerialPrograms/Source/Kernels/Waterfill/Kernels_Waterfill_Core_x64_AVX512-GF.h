/*  Waterfill Core (AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H
#define PokemonAutomation_Kernels_Waterfill_Core_x64_AVX512GF_H

#include "Kernels_Waterfill_Core_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


template <> PA_FORCE_INLINE __m512i bit_reverse<true>(__m512i x){
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );
    return _mm512_gf2p8affine_epi64_epi8(x, _mm512_set1_epi64(0x8040201008040201), 0);
}




struct Waterfill_x64_AVX512GF : public Waterfill_x64_AVX512{



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
static void Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    Waterfill::Waterfill_expand<true>(m, x);
}




};



}
}
}
#endif
