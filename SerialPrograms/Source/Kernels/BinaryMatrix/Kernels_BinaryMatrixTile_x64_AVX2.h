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


public:
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


public:
    PA_FORCE_INLINE void set_zero(){
        vec[0] = _mm256_setzero_si256();
        vec[1] = _mm256_setzero_si256();
        vec[2] = _mm256_setzero_si256();
        vec[3] = _mm256_setzero_si256();
    }
    PA_FORCE_INLINE void set_ones(){
        vec[0] = _mm256_set1_epi32(0xffffffff);
        vec[1] = _mm256_set1_epi32(0xffffffff);
        vec[2] = _mm256_set1_epi32(0xffffffff);
        vec[3] = _mm256_set1_epi32(0xffffffff);
    }
    PA_FORCE_INLINE void set_ones(size_t width, size_t height){
        __m256i word = _mm256_set1_epi64x(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m256i vheight = _mm256_set1_epi64x(height);
        __m256i index = _mm256_setr_epi64x(0, 4, 8, 12);
        for (size_t c = 0; c < 4; c++){
            __m256i mask = _mm256_cmpgt_epi64(vheight, index);
            vec[c] = _mm256_and_si256(mask, word);
            index = _mm256_add_epi64(index, _mm256_set1_epi64x(1));
        }
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_AVX2& x){
        vec[0] = _mm256_xor_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_xor_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_xor_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_xor_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_AVX2& x){
        vec[0] = _mm256_or_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_or_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_or_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_or_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_AVX2& x){
        vec[0] = _mm256_and_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_and_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_and_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_and_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_AVX2& x){
        vec[0] = _mm256_andnot_si256(x.vec[0], vec[0]);
        vec[1] = _mm256_andnot_si256(x.vec[1], vec[1]);
        vec[2] = _mm256_andnot_si256(x.vec[2], vec[2]);
        vec[3] = _mm256_andnot_si256(x.vec[3], vec[3]);
    }


public:
    uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    uint64_t bottom() const{
        return ((const uint64_t*)vec)[15];
    }
    uint64_t& bottom(){
        return ((uint64_t*)vec)[15];
    }

    uint64_t row(size_t index) const{
        index = ((index & 3) << 2) | (index >> 2);
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 3) << 2) | (index >> 2);
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
        for (size_t c = 0; c < 16; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
