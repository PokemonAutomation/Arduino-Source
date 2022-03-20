/*  Binary Matrix
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */


#include "Common/Cpp/CpuId.h"
#include "Kernels_PackedBinaryMatrixCore.tpp"
#include "Kernels_SparseBinaryMatrixCore.tpp"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{



BinaryMatrixType get_BinaryMatrixType(){
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return BinaryMatrixType::AVX512;
    }
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return BinaryMatrixType::AVX2;
    }
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return BinaryMatrixType::SSE42;
    }
    return BinaryMatrixType::Default;
}


std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_Default();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_Default(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_SSE42();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_SSE42(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX2();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX2(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX512();
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix_x64_AVX512(size_t width, size_t height);

std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type){
    switch (type){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        return make_PackedBinaryMatrix_x64_AVX512();
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_PackedBinaryMatrix_x64_AVX2();
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_PackedBinaryMatrix_x64_SSE42();
#endif
    default:
        return make_PackedBinaryMatrix_Default();
    }
}
std::unique_ptr<PackedBinaryMatrix_IB> make_PackedBinaryMatrix(BinaryMatrixType type, size_t width, size_t height){
    switch (type){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        return make_PackedBinaryMatrix_x64_AVX512(width, height);
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_PackedBinaryMatrix_x64_AVX2(width, height);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_PackedBinaryMatrix_x64_SSE42(width, height);
#endif
    default:
        return make_PackedBinaryMatrix_Default(width, height);
    }
}


std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_Default();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_Default(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_SSE42();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_SSE42(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX2();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX2(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX512();
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix_x64_AVX512(size_t width, size_t height);

std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type){
    switch (type){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        return make_SparseBinaryMatrix_x64_AVX512();
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_SparseBinaryMatrix_x64_AVX2();
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_SparseBinaryMatrix_x64_SSE42();
#endif
    default:
        return make_SparseBinaryMatrix_Default();
    }
}
std::unique_ptr<SparseBinaryMatrix_IB> make_SparseBinaryMatrix(BinaryMatrixType type, size_t width, size_t height){
    switch (type){
#ifdef PA_AutoDispatch_17_Skylake
    case BinaryMatrixType::AVX512:
        return make_SparseBinaryMatrix_x64_AVX512(width, height);
#endif
#ifdef PA_AutoDispatch_13_Haswell
    case BinaryMatrixType::AVX2:
        return make_SparseBinaryMatrix_x64_AVX2(width, height);
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    case BinaryMatrixType::SSE42:
        return make_SparseBinaryMatrix_x64_SSE42(width, height);
#endif
    default:
        return make_SparseBinaryMatrix_Default(width, height);
    }
}




}
}
