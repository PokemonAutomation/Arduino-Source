/*  Waterfill Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_Waterfill_Routines.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


std::unique_ptr<WaterfillSession> make_WaterfillSession_64x4_Default        (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_Default        (PackedBinaryMatrix_IB* matrix);

std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_x64_SSE42      (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x16_x64_AVX2      (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x32_x64_AVX512    (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x32_x64_AVX512GF  (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x64_x64_AVX512    (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x64_x64_AVX512GF  (PackedBinaryMatrix_IB* matrix);
std::unique_ptr<WaterfillSession> make_WaterfillSession_64x8_arm64_NEON      (PackedBinaryMatrix_IB* matrix);

std::unique_ptr<WaterfillSession> make_WaterfillSession(){
//    cout << "make_WaterfillSession()" << endl;

    BinaryMatrixType type = get_BinaryMatrixType();
    switch (type){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillSession_64x64_x64_AVX512GF(nullptr);
        }else{
            return make_WaterfillSession_64x64_x64_AVX512(nullptr);
        }
    case BinaryMatrixType::i64x32_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillSession_64x32_x64_AVX512GF(nullptr);
        }else{
            return make_WaterfillSession_64x32_x64_AVX512(nullptr);
        }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_WaterfillSession_64x16_x64_AVX2(nullptr);
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_WaterfillSession_64x8_x64_SSE42(nullptr);
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_WaterfillSession_64x8_arm64_NEON(nullptr);
#endif
    case BinaryMatrixType::i64x8_Default:
        return make_WaterfillSession_64x8_Default(nullptr);
    case BinaryMatrixType::i64x4_Default:
        return make_WaterfillSession_64x4_Default(nullptr);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}
std::unique_ptr<WaterfillSession> make_WaterfillSession(PackedBinaryMatrix_IB& matrix){
//    cout << "make_WaterfillSession(PackedBinaryMatrix_IB& matrix)" << endl;

    switch (matrix.type()){
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillSession_64x64_x64_AVX512GF(&matrix);
        }else{
            return make_WaterfillSession_64x64_x64_AVX512(&matrix);
        }
    case BinaryMatrixType::i64x32_x64_AVX512:
        if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
            return make_WaterfillSession_64x32_x64_AVX512GF(&matrix);
        }else{
            return make_WaterfillSession_64x32_x64_AVX512(&matrix);
        }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_WaterfillSession_64x16_x64_AVX2(&matrix);
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_WaterfillSession_64x8_x64_SSE42(&matrix);
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_WaterfillSession_64x8_arm64_NEON(&matrix);
#endif
    case BinaryMatrixType::i64x8_Default:
        return make_WaterfillSession_64x8_Default(&matrix);
    case BinaryMatrixType::i64x4_Default:
        return make_WaterfillSession_64x4_Default(&matrix);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}



}
}
}
