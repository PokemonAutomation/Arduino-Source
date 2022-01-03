/*  Binary Matrix Tile (SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_SSE41_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_SSE41_H

#include <nmmintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_SSE42{
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 8;

    __m128i vec[4];

public:
    PA_FORCE_INLINE BinaryTile_SSE42() = default;
    PA_FORCE_INLINE BinaryTile_SSE42(const BinaryTile_SSE42& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_SSE42& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }


public:
    PA_FORCE_INLINE void set_zero(){
        vec[0] = _mm_setzero_si128();
        vec[1] = _mm_setzero_si128();
        vec[2] = _mm_setzero_si128();
        vec[3] = _mm_setzero_si128();
    }
    PA_FORCE_INLINE void set_ones(){
        vec[0] = _mm_set1_epi32(0xffffffff);
        vec[1] = _mm_set1_epi32(0xffffffff);
        vec[2] = _mm_set1_epi32(0xffffffff);
        vec[3] = _mm_set1_epi32(0xffffffff);
    }
    PA_FORCE_INLINE void set_ones(size_t width, size_t height){
        __m128i word = _mm_set1_epi64x(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m128i vheight = _mm_set1_epi64x(height);
        __m128i mask;
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(2, 0));
        vec[0] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(3, 1));
        vec[1] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(6, 4));
        vec[2] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(7, 5));
        vec[3] = _mm_and_si128(mask, word);
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_SSE42& x){
        vec[0] = _mm_xor_si128(vec[0], x.vec[0]);
        vec[1] = _mm_xor_si128(vec[1], x.vec[1]);
        vec[2] = _mm_xor_si128(vec[2], x.vec[2]);
        vec[3] = _mm_xor_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_SSE42& x){
        vec[0] = _mm_or_si128(vec[0], x.vec[0]);
        vec[1] = _mm_or_si128(vec[1], x.vec[1]);
        vec[2] = _mm_or_si128(vec[2], x.vec[2]);
        vec[3] = _mm_or_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_SSE42& x){
        vec[0] = _mm_and_si128(vec[0], x.vec[0]);
        vec[1] = _mm_and_si128(vec[1], x.vec[1]);
        vec[2] = _mm_and_si128(vec[2], x.vec[2]);
        vec[3] = _mm_and_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_SSE42& x){
        vec[0] = _mm_andnot_si128(x.vec[0], vec[0]);
        vec[1] = _mm_andnot_si128(x.vec[1], vec[1]);
        vec[2] = _mm_andnot_si128(x.vec[2], vec[2]);
        vec[3] = _mm_andnot_si128(x.vec[3], vec[3]);
    }


public:
    uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    uint64_t bottom() const{
        return ((const uint64_t*)vec)[7];
    }
    uint64_t& bottom(){
        return ((uint64_t*)vec)[7];
    }

    uint64_t row(size_t index) const{
        index = ((index & 2) >> 1) | ((index & 1) << 1) | ((index & 4));
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 2) >> 1) | ((index & 1) << 1) | ((index & 4));
        return ((uint64_t*)vec)[index];
    }

    //  These are slow.
    bool get_bit(size_t x, size_t y) const{
        return (row(y) >> x) & 1;
    }
    void set_bit(size_t x, size_t y, bool set){
        uint64_t bit = (uint64_t)(set ? 1 : 0) << x;
        uint64_t mask = (uint64_t)1 << x;
        uint64_t& word = row(y);
        word = (word & ~mask) | bit;
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
