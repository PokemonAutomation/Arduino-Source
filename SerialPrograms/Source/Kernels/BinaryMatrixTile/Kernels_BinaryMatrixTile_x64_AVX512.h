/*  Binary Matrix Tile (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_AVX512_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_AVX512_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_AVX512{
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 64;

    __m512i vec[8];

    PA_FORCE_INLINE BinaryTile_AVX512() = default;
    PA_FORCE_INLINE BinaryTile_AVX512(const BinaryTile_AVX512& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
        vec[4] = x.vec[4];
        vec[5] = x.vec[5];
        vec[6] = x.vec[6];
        vec[7] = x.vec[7];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_AVX512& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
        vec[4] = x.vec[4];
        vec[5] = x.vec[5];
        vec[6] = x.vec[6];
        vec[7] = x.vec[7];
    }

    PA_FORCE_INLINE void set_zero(){
        vec[0] = _mm512_setzero_si512();
        vec[1] = _mm512_setzero_si512();
        vec[2] = _mm512_setzero_si512();
        vec[3] = _mm512_setzero_si512();
        vec[4] = _mm512_setzero_si512();
        vec[5] = _mm512_setzero_si512();
        vec[6] = _mm512_setzero_si512();
        vec[7] = _mm512_setzero_si512();
    }
    uint64_t row(size_t index) const{
        index = ((index & 7) << 3) | (index >> 3);
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 7) << 3) | (index >> 3);
        return ((uint64_t*)vec)[index];
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 64; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
