/*  Binary Matrix Tile (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_64x16_x64_AVX2_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_64x16_x64_AVX2_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_64x16_x64_AVX2{
    static constexpr BinaryMatrixType TYPE = BinaryMatrixType::i64x16_x64_AVX2;
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 16;

    __m256i vec[4];


public:
    PA_FORCE_INLINE BinaryTile_64x16_x64_AVX2(){
        set_zero();
    }
    PA_FORCE_INLINE BinaryTile_64x16_x64_AVX2(const BinaryTile_64x16_x64_AVX2& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_64x16_x64_AVX2& x){
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
        __m256i index = _mm256_setr_epi64x(0, 1, 2, 3);
        for (size_t c = 0; c < 4; c++){
            __m256i mask = _mm256_cmpgt_epi64(vheight, index);
            vec[c] = _mm256_and_si256(mask, word);
            index = _mm256_add_epi64(index, _mm256_set1_epi64x(4));
        }
    }
    PA_FORCE_INLINE void clear_padding(size_t width, size_t height){
        __m256i word = _mm256_set1_epi64x(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m256i vheight = _mm256_set1_epi64x(height);
        __m256i index = _mm256_setr_epi64x(0, 1, 2, 3);
        for (size_t c = 0; c < 4; c++){
            __m256i mask = _mm256_cmpgt_epi64(vheight, index);
            mask = _mm256_and_si256(mask, word);
            vec[c] = _mm256_and_si256(vec[c], mask);
            index = _mm256_add_epi64(index, _mm256_set1_epi64x(4));
        }
    }
    PA_FORCE_INLINE void invert(){
        vec[0] = _mm256_xor_si256(vec[0], _mm256_set1_epi32(0xffffffff));
        vec[1] = _mm256_xor_si256(vec[1], _mm256_set1_epi32(0xffffffff));
        vec[2] = _mm256_xor_si256(vec[2], _mm256_set1_epi32(0xffffffff));
        vec[3] = _mm256_xor_si256(vec[3], _mm256_set1_epi32(0xffffffff));
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_64x16_x64_AVX2& x){
        vec[0] = _mm256_xor_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_xor_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_xor_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_xor_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_64x16_x64_AVX2& x){
        vec[0] = _mm256_or_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_or_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_or_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_or_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_64x16_x64_AVX2& x){
        vec[0] = _mm256_and_si256(vec[0], x.vec[0]);
        vec[1] = _mm256_and_si256(vec[1], x.vec[1]);
        vec[2] = _mm256_and_si256(vec[2], x.vec[2]);
        vec[3] = _mm256_and_si256(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_64x16_x64_AVX2& x){
        vec[0] = _mm256_andnot_si256(x.vec[0], vec[0]);
        vec[1] = _mm256_andnot_si256(x.vec[1], vec[1]);
        vec[2] = _mm256_andnot_si256(x.vec[2], vec[2]);
        vec[3] = _mm256_andnot_si256(x.vec[3], vec[3]);
    }


public:
    PA_FORCE_INLINE uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t bottom() const{
        return ((const uint64_t*)vec)[15];
    }
    PA_FORCE_INLINE uint64_t& bottom(){
        return ((uint64_t*)vec)[15];
    }

    PA_FORCE_INLINE uint64_t row(size_t index) const{
        return ((const uint64_t*)vec)[index];
    }
    PA_FORCE_INLINE uint64_t& row(size_t index){
        return ((uint64_t*)vec)[index];
    }

    //  These are slow.
    bool get_bit(size_t x, size_t y) const{
        return (row(y) >> x) & 1;
    }
    void set_bit(size_t x, size_t y){
        row(y) |= (uint64_t)1 << x;
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


public:
    //  Copy the current tile into "tile" while applying the specified shifts.
    //  These are used to implement submatrix extraction where the desired
    //  sub-matrix may of arbitrary shift and alignment.

    void copy_to_shift_pp(BinaryTile_64x16_x64_AVX2& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, +y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 13){
            __m256i r0 = _mm256_loadu_si256((const __m256i*)(src + shift_y));
            r0 = _mm256_srl_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)dest));
            _mm256_store_si256((__m256i*)dest, r0);
            dest += 4;
            shift_y += 4;
        }
        if (shift_y < 16){
            __m256i mask = _mm256_set1_epi64x(shift_y);
            mask = _mm256_cmpgt_epi64(_mm256_setr_epi64x(16, 15, 14, 13), mask);
            __m256i r0 = _mm256_maskload_epi64((const long long*)(src + shift_y), mask);
            r0 = _mm256_srl_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)dest));
            _mm256_store_si256((__m256i*)dest, r0);
        }
    }
    void copy_to_shift_np(BinaryTile_64x16_x64_AVX2& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, +y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 13){
            __m256i r0 = _mm256_loadu_si256((const __m256i*)(src + shift_y));
            r0 = _mm256_sll_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)dest));
            _mm256_store_si256((__m256i*)dest, r0);
            dest += 4;
            shift_y += 4;
        }
        if (shift_y < 16){
            __m256i mask = _mm256_set1_epi64x(shift_y);
            mask = _mm256_cmpgt_epi64(_mm256_setr_epi64x(16, 15, 14, 13), mask);
            __m256i r0 = _mm256_maskload_epi64((const long long*)(src + shift_y), mask);
            r0 = _mm256_sll_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)dest));
            _mm256_store_si256((__m256i*)dest, r0);
        }
    }
    void copy_to_shift_pn(BinaryTile_64x16_x64_AVX2& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, -y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        size_t align = (16 - shift_y) & 3;
        if (align){
            src += align - 4;
            shift_y += align - 4;
            __m256i mask = _mm256_set1_epi64x(align);
            mask = _mm256_cmpgt_epi64(mask, _mm256_setr_epi64x(3, 2, 1, 0));
            __m256i r0 = _mm256_maskload_epi64((const long long*)src, mask);
            r0 = _mm256_srl_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)(dest + shift_y)));
            _mm256_store_si256((__m256i*)(dest + shift_y), r0);
            src += 4;
            shift_y += 4;
        }
        while (shift_y < 16){
            __m256i r0 = _mm256_loadu_si256((const __m256i*)src);
            r0 = _mm256_srl_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)(dest + shift_y)));
            _mm256_store_si256((__m256i*)(dest + shift_y), r0);
            src += 4;
            shift_y += 4;
        }
    }
    void copy_to_shift_nn(BinaryTile_64x16_x64_AVX2& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, -y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        size_t align = (16 - shift_y) & 3;
        if (align){
            src += align - 4;
            shift_y += align - 4;
            __m256i mask = _mm256_set1_epi64x(align);
            mask = _mm256_cmpgt_epi64(mask, _mm256_setr_epi64x(3, 2, 1, 0));
            __m256i r0 = _mm256_maskload_epi64((const long long*)src, mask);
            r0 = _mm256_sll_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)(dest + shift_y)));
            _mm256_store_si256((__m256i*)(dest + shift_y), r0);
            src += 4;
            shift_y += 4;
        }
        while (shift_y < 16){
            __m256i r0 = _mm256_loadu_si256((const __m256i*)src);
            r0 = _mm256_sll_epi64(r0, shift);
            r0 = _mm256_or_si256(r0, _mm256_load_si256((__m256i*)(dest + shift_y)));
            _mm256_store_si256((__m256i*)(dest + shift_y), r0);
            src += 4;
            shift_y += 4;
        }
    }
};





}
}
#endif
