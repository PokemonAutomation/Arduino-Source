/*  Binary Matrix Tile (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_AVX2_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_AVX2_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_AVX2{
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 16;

    __m256i vec[4];

    PA_FORCE_INLINE BinaryTile_AVX2() = default;
    PA_FORCE_INLINE BinaryTile_AVX2(const BinaryTile_AVX2& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_AVX2& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }

    PA_FORCE_INLINE void set_zero(){
        vec[0] = _mm256_setzero_si256();
        vec[1] = _mm256_setzero_si256();
        vec[2] = _mm256_setzero_si256();
        vec[3] = _mm256_setzero_si256();
    }
    uint64_t row(size_t index) const{
        index = ((index & 3) << 2) | (index >> 2);
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 3) << 2) | (index >> 2);
        return ((uint64_t*)vec)[index];
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 16; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
