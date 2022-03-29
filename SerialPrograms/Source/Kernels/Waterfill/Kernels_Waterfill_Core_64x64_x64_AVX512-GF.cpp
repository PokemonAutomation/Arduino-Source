/*  Waterfill Core (AVX512-GF)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_19_IceLake

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_64x64_x64_AVX512-GF.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::vector<WaterfillObject> find_objects_inplace_x64_AVX512GF(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return find_objects_inplace<BinaryTile_64x64_x64_AVX512, Waterfill_x64_AVX512GF>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        min_area
    );
}
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX512GF(PackedBinaryMatrix_IB* matrix){
    return matrix == nullptr
        ? std::make_unique<WaterfillSession_t<BinaryTile_64x64_x64_AVX512, Waterfill_x64_AVX512GF>>()
        : std::make_unique<WaterfillSession_t<BinaryTile_64x64_x64_AVX512, Waterfill_x64_AVX512GF>>(
            static_cast<PackedBinaryMatrix_x64_AVX512*>(matrix)->get()
        );
}



}
}
}
#endif
