/*  Waterfill Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/CpuId.h"
#include "Kernels_Waterfill_Routines.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::unique_ptr<WaterfillSession> make_WaterfillSession_Default     (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_SSE42   (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX2    (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX512  (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_x64_AVX512GF(PackedBinaryMatrix_IB* matrix);

std::unique_ptr<WaterfillSession> make_WaterfillSession(){
#ifdef PA_AutoDispatch_19_IceLake
    if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
        return make_WaterfillSession_x64_AVX512GF(nullptr);
    }
#endif
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return make_WaterfillSession_x64_AVX512(nullptr);
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return make_WaterfillSession_x64_AVX2(nullptr);
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return make_WaterfillSession_x64_SSE42(nullptr);
    }
#endif
    return make_WaterfillSession_Default(nullptr);
}
std::unique_ptr<WaterfillSession> make_WaterfillSession(PackedBinaryMatrix_IB& matrix){
    switch (matrix.type()){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillSession_x64_AVX512GF(&matrix);
        }else{
            return make_WaterfillSession_x64_AVX512(&matrix);
        }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_WaterfillSession_x64_AVX2(&matrix);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_WaterfillSession_x64_SSE42(&matrix);
#endif
    default:
        return make_WaterfillSession_Default(&matrix);
    }
}



}
}
}
