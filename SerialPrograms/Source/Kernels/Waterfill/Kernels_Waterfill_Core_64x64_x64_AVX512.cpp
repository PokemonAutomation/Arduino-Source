/*  Waterfill Core (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x64_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::vector<WaterfillObject> find_objects_inplace_64x64_x64_AVX512(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x64_x64_AVX512, Waterfill_64x64_x64_AVX512>(
        static_cast<PackedBinaryMatrix_64x64_x64_AVX512&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x64_x64_AVX512(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x64_x64_AVX512, Waterfill_64x64_x64_AVX512>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x64_x64_AVX512, Waterfill_64x64_x64_AVX512>>(
            static_cast<PackedBinaryMatrix_64x64_x64_AVX512*>(matrix)->get()
        );
}




}
}
}
#endif
