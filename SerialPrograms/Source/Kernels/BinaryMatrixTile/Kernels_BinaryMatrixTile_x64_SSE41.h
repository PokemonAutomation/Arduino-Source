/*  Binary Matrix Tile (SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_SSE41_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_SSE41_H

#include <smmintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_SSE41{
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 8;

    __m128i vec[4];

    PA_FORCE_INLINE BinaryTile_SSE41() = default;
    PA_FORCE_INLINE BinaryTile_SSE41(const BinaryTile_SSE41& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_SSE41& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }

    PA_FORCE_INLINE void set_zero(){
        vec[0] = _mm_setzero_si128();
        vec[1] = _mm_setzero_si128();
        vec[2] = _mm_setzero_si128();
        vec[3] = _mm_setzero_si128();
    }
    uint64_t row(size_t index) const{
        index = ((index & 2) >> 1) | ((index & 1) << 1) | ((index & 4));
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 2) >> 1) | ((index & 1) << 1) | ((index & 4));
        return ((uint64_t*)vec)[index];
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 8; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
