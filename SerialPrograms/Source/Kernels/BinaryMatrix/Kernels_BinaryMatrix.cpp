/*  Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{



BinaryMatrixType get_BinaryMatrixType(){

#ifdef PA_ARCH_x86
//    if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
//        return BinaryMatrixType::i64x32_x64_AVX512;
//    }
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return BinaryMatrixType::i64x32_x64_AVX512;
    }
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return BinaryMatrixType::i64x16_x64_AVX2;
    }
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return BinaryMatrixType::i64x8_x64_SSE42;
    }
#elif PA_ARCH_arm64
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return BinaryMatrixType::arm64x8_x64_NEON;
    }
#endif

//    return BinaryMatrixType::i64x8_Default;
    return BinaryMatrixType::i64x4_Default;
}


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x4_Default();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x4_Default(size_t width, size_t height);
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_Default();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_Default(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_x64_SSE42();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_x64_SSE42(size_t width, size_t height);
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x16_x64_AVX2();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x16_x64_AVX2(size_t width, size_t height);
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x32_x64_AVX512();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x32_x64_AVX512(size_t width, size_t height);
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x64_x64_AVX512();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x64_x64_AVX512(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_arm64_NEON();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_64x8_arm64_NEON(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type){
    switch (type){

#ifdef PA_ARCH_x86
#ifdef PA_AutoDispatch_x64_19_IceLake
    case BinaryMatrixType::i64x32_x64_AVX512:
        return make_PackedBinaryMatrix_64x32_x64_AVX512();
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        return make_PackedBinaryMatrix_64x64_x64_AVX512();
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_PackedBinaryMatrix_64x16_x64_AVX2();
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_PackedBinaryMatrix_64x8_x64_SSE42();
#endif
#elif PA_ARCH_arm64
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_PackedBinaryMatrix_64x8_arm64_NEON();
#endif
#endif

    case BinaryMatrixType::i64x8_Default:
        return make_PackedBinaryMatrix_64x8_Default();
    case BinaryMatrixType::i64x4_Default:
        return make_PackedBinaryMatrix_64x4_Default();
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type, size_t width, size_t height){
    switch (type){

#ifdef PA_ARCH_x86
#ifdef PA_AutoDispatch_x64_19_IceLake
    case BinaryMatrixType::i64x32_x64_AVX512:
        return make_PackedBinaryMatrix_64x32_x64_AVX512(width, height);
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        return make_PackedBinaryMatrix_64x64_x64_AVX512(width, height);
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_PackedBinaryMatrix_64x16_x64_AVX2(width, height);
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_PackedBinaryMatrix_64x8_x64_SSE42(width, height);
#endif
#elif PA_ARCH_arm64
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_PackedBinaryMatrix_64x8_arm64_NEON(width, height);
#endif
#endif

    case BinaryMatrixType::i64x8_Default:
        return make_PackedBinaryMatrix_64x8_Default(width, height);
    case BinaryMatrixType::i64x4_Default:
        return make_PackedBinaryMatrix_64x4_Default(width, height);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}


std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x4_Default();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x4_Default(size_t width, size_t height);
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_Default();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_Default(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_x64_SSE42();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_x64_SSE42(size_t width, size_t height);
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x16_x64_AVX2();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x16_x64_AVX2(size_t width, size_t height);
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x32_x64_AVX512();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x32_x64_AVX512(size_t width, size_t height);
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x64_x64_AVX512();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x64_x64_AVX512(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_arm64_NEON();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_64x8_arm64_NEON(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type){
    switch (type){

#ifdef PA_ARCH_x86
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        return make_SparseBinaryMatrix_64x64_x64_AVX512();
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_SparseBinaryMatrix_64x16_x64_AVX2();
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_SparseBinaryMatrix_64x8_x64_SSE42();
#endif
#elif PA_ARCH_arm64
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_SparseBinaryMatrix_64x8_arm64_NEON();
#endif
#endif

    case BinaryMatrixType::i64x8_Default:
        return make_SparseBinaryMatrix_64x8_Default();
    case BinaryMatrixType::i64x4_Default:
        return make_SparseBinaryMatrix_64x4_Default();
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type, size_t width, size_t height){
    switch (type){

#ifdef PA_ARCH_x86
#ifdef PA_AutoDispatch_x64_17_Skylake
    case BinaryMatrixType::i64x64_x64_AVX512:
        return make_SparseBinaryMatrix_64x64_x64_AVX512(width, height);
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    case BinaryMatrixType::i64x16_x64_AVX2:
        return make_SparseBinaryMatrix_64x16_x64_AVX2(width, height);
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    case BinaryMatrixType::i64x8_x64_SSE42:
        return make_SparseBinaryMatrix_64x8_x64_SSE42(width, height);
#endif
#elif PA_ARCH_arm64
#ifdef PA_AutoDispatch_arm64_20_M1
    case BinaryMatrixType::arm64x8_x64_NEON:
        return make_SparseBinaryMatrix_64x8_arm64_NEON(width, height);
#endif
#endif

    case BinaryMatrixType::i64x8_Default:
        return make_SparseBinaryMatrix_64x8_Default(width, height);
    case BinaryMatrixType::i64x4_Default:
        return make_SparseBinaryMatrix_64x4_Default(width, height);
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Unsupported tile type.");
    }
}




}
}
