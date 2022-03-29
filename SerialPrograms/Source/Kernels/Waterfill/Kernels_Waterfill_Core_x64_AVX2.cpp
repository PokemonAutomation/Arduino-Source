/*  Waterfill Core (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_13_Haswell

#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::vector<WaterfillObject> find_objects_inplace_x64_AVX2(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x16_x64_AVX2, Waterfill_x64_AVX2>(
        static_cast<PackedBinaryMatrix_x64_AVX2&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX2(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x16_x64_AVX2, Waterfill_x64_AVX2>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x16_x64_AVX2, Waterfill_x64_AVX2>>(
            static_cast<PackedBinaryMatrix_x64_AVX2*>(matrix)->get()
        );
}




}
}
}
#endif
