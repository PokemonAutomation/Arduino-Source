/*  Waterfill Core (64-bit integer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_Waterfill_Session.tpp"
#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x4_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{




std::vector<WaterfillObject> find_objects_inplace_64x4_Default(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x4_Default, Waterfill_64x4_Default>(
        static_cast<PackedBinaryMatrix_64x4_Default&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x4_Default(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x4_Default, Waterfill_64x4_Default>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x4_Default, Waterfill_64x4_Default>>(
            static_cast<PackedBinaryMatrix_64x4_Default*>(matrix)->get()
        );
}







}
}
}
