/*  Waterfill Core (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x8_x64_SSE42.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::vector<WaterfillObject> find_objects_inplace_64x8_x64_SSE42(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x8_x64_SSE42, Waterfill_64x8_x64_SSE42>(
        static_cast<PackedBinaryMatrix_64x8_x64_SSE42&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_x64_SSE42(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x8_x64_SSE42, Waterfill_64x8_x64_SSE42>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x8_x64_SSE42, Waterfill_64x8_x64_SSE42>>(
            static_cast<PackedBinaryMatrix_64x8_x64_SSE42*>(matrix)->get()
        );
}




}
}
}
#endif
