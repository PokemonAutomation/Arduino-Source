/*  Waterfill Core (AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_19_IceLake

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_x64_AVX512-GF.h"

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


void Waterfill_x64_AVX512GF::Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    Waterfill::Waterfill_expand<true>(m, x);
}



std::vector<WaterfillObject> find_objects_inplace_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_AVX512, Waterfill_x64_AVX512GF>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX512GF(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_AVX512, Waterfill_x64_AVX512GF>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_AVX512, Waterfill_x64_AVX512GF>>(
            static_cast<PackedBinaryMatrix_x64_AVX512*>(matrix)->get()
        );
}
std::unique_ptr<WaterfillIterator> make_WaterfillIterator_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return std::make_unique<WaterfillIterator_TI<BinaryTile_AVX512, Waterfill_x64_AVX512GF>>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        min_area
    );
}


}
}
}
#endif
