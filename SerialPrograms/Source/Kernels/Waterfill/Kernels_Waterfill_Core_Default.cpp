/*  Waterfill Core (64-bit integer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_Waterfill_Session_TI.h"
#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{




std::vector<WaterfillObject> find_objects_inplace_Default(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_Default, Waterfill_Default>(
        static_cast<PackedBinaryMatrix_Default&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_Default(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_Default, Waterfill_Default>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_Default, Waterfill_Default>>(
            static_cast<PackedBinaryMatrix_Default*>(matrix)->get()
        );
}







}
}
}
