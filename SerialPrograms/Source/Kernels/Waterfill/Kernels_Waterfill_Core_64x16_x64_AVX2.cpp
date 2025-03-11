/*  Waterfill Core (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x16_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::vector<WaterfillObject> find_objects_inplace_64x16_x64_AVX2(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x16_x64_AVX2, Waterfill_64x16_x64_AVX2>(
        static_cast<PackedBinaryMatrix_64x16_x64_AVX2&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x16_x64_AVX2(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x16_x64_AVX2, Waterfill_64x16_x64_AVX2>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x16_x64_AVX2, Waterfill_64x16_x64_AVX2>>(
            static_cast<PackedBinaryMatrix_64x16_x64_AVX2*>(matrix)->get()
        );
}




}
}
}
#endif
