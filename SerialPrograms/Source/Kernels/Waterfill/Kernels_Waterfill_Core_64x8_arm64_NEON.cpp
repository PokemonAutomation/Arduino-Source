/*  Waterfill Core (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

// #define USE_CPP_TEMPLATE_IMPL

#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x8_arm64_NEON.h"

#ifdef USE_CPP_TEMPLATE_IMPL
#include "Kernels_Waterfill_Core_64xH_Default.h"
#endif

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{

#ifdef USE_CPP_TEMPLATE_IMPL

using Waterfill_64x8_Default = Waterfill_64xH_Default<BinaryTile_64x8_arm64_NEON>;

std::vector<WaterfillObject> find_objects_inplace_64x8_arm64_NEON(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_Default>(
        static_cast<PackedBinaryMatrix_64x8_arm64_NEON&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_arm64_NEON(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_Default>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_Default>>(
            static_cast<PackedBinaryMatrix_64x8_arm64_NEON*>(matrix)->get()
        );
}

#else

std::vector<WaterfillObject> find_objects_inplace_64x8_arm64_NEON(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_arm64_NEON>(
        static_cast<PackedBinaryMatrix_64x8_arm64_NEON&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_arm64_NEON(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_arm64_NEON>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x8_arm64_NEON, Waterfill_64x8_arm64_NEON>>(
            static_cast<PackedBinaryMatrix_64x8_arm64_NEON*>(matrix)->get()
        );
}

#endif


}
}
}
#endif
