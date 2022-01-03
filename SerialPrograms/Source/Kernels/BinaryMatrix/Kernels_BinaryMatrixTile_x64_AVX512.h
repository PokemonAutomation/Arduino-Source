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

public:
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


public:
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
    PA_FORCE_INLINE void set_ones(){
        vec[0] = _mm512_set1_epi32(0xffffffff);
        vec[1] = _mm512_set1_epi32(0xffffffff);
        vec[2] = _mm512_set1_epi32(0xffffffff);
        vec[3] = _mm512_set1_epi32(0xffffffff);
        vec[4] = _mm512_set1_epi32(0xffffffff);
        vec[5] = _mm512_set1_epi32(0xffffffff);
        vec[6] = _mm512_set1_epi32(0xffffffff);
        vec[7] = _mm512_set1_epi32(0xffffffff);
    }
    PA_FORCE_INLINE void set_ones(size_t width, size_t height){
        __m512i word = _mm512_set1_epi64(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m512i vheight = _mm512_set1_epi64(height);
        __m512i index = _mm512_setr_epi64(0, 8, 16, 24, 32, 40, 48, 56);
        for (size_t c = 0; c < 8; c++){
            __mmask8 mask = _mm512_cmplt_epi64_mask(index, vheight);
            vec[c] = _mm512_maskz_mov_epi64(mask, word);
            index = _mm512_add_epi64(index, _mm512_set1_epi64(1));
        }
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_AVX512& x){
        vec[0] = _mm512_xor_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_xor_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_xor_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_xor_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_xor_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_xor_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_xor_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_xor_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_AVX512& x){
        vec[0] = _mm512_or_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_or_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_or_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_or_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_or_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_or_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_or_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_or_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_AVX512& x){
        vec[0] = _mm512_and_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_and_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_and_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_and_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_and_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_and_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_and_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_and_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_AVX512& x){
        vec[0] = _mm512_andnot_si512(x.vec[0], vec[0]);
        vec[1] = _mm512_andnot_si512(x.vec[1], vec[1]);
        vec[2] = _mm512_andnot_si512(x.vec[2], vec[2]);
        vec[3] = _mm512_andnot_si512(x.vec[3], vec[3]);
        vec[4] = _mm512_andnot_si512(x.vec[4], vec[4]);
        vec[5] = _mm512_andnot_si512(x.vec[5], vec[5]);
        vec[6] = _mm512_andnot_si512(x.vec[6], vec[6]);
        vec[7] = _mm512_andnot_si512(x.vec[7], vec[7]);
    }


public:
    uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    uint64_t bottom() const{
        return ((const uint64_t*)vec)[63];
    }
    uint64_t& bottom(){
        return ((uint64_t*)vec)[63];
    }

    uint64_t row(size_t index) const{
        index = ((index & 7) << 3) | (index >> 3);
        return ((const uint64_t*)vec)[index];
    }
    uint64_t& row(size_t index){
        index = ((index & 7) << 3) | (index >> 3);
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
        for (size_t c = 0; c < 64; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }
};





}
}
#endif
