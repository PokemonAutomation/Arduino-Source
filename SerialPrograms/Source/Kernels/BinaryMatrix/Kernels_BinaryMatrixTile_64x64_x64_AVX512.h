/*  Binary Matrix Tile (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_64x64_x64_AVX512_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_64x64_x64_AVX512_H

#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_64x64_x64_AVX512{
    static constexpr BinaryMatrixType TYPE = BinaryMatrixType::i64x64_x64_AVX512;
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 64;

    __m512i vec[8];

public:
    PA_FORCE_INLINE BinaryTile_64x64_x64_AVX512(){
        set_zero();
    }
    PA_FORCE_INLINE BinaryTile_64x64_x64_AVX512(const BinaryTile_64x64_x64_AVX512& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
        vec[4] = x.vec[4];
        vec[5] = x.vec[5];
        vec[6] = x.vec[6];
        vec[7] = x.vec[7];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_64x64_x64_AVX512& x){
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
        __m512i index = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        for (size_t c = 0; c < 8; c++){
            __mmask8 mask = _mm512_cmplt_epi64_mask(index, vheight);
            vec[c] = _mm512_maskz_mov_epi64(mask, word);
            index = _mm512_add_epi64(index, _mm512_set1_epi64(8));
        }
    }
    PA_FORCE_INLINE void clear_padding(size_t width, size_t height){
        __m512i word = _mm512_set1_epi64(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m512i vheight = _mm512_set1_epi64(height);
        __m512i index = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
        for (size_t c = 0; c < 8; c++){
            __mmask8 mask = _mm512_cmplt_epi64_mask(index, vheight);
            __m512i maskv = _mm512_maskz_mov_epi64(mask, word);
            vec[c] = _mm512_and_si512(vec[c], maskv);
            index = _mm512_add_epi64(index, _mm512_set1_epi64(8));
        }
    }
    PA_FORCE_INLINE void invert(){
        vec[0] = _mm512_xor_si512(vec[0], _mm512_set1_epi32(0xffffffff));
        vec[1] = _mm512_xor_si512(vec[1], _mm512_set1_epi32(0xffffffff));
        vec[2] = _mm512_xor_si512(vec[2], _mm512_set1_epi32(0xffffffff));
        vec[3] = _mm512_xor_si512(vec[3], _mm512_set1_epi32(0xffffffff));
        vec[4] = _mm512_xor_si512(vec[4], _mm512_set1_epi32(0xffffffff));
        vec[5] = _mm512_xor_si512(vec[5], _mm512_set1_epi32(0xffffffff));
        vec[6] = _mm512_xor_si512(vec[6], _mm512_set1_epi32(0xffffffff));
        vec[7] = _mm512_xor_si512(vec[7], _mm512_set1_epi32(0xffffffff));
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_64x64_x64_AVX512& x){
        vec[0] = _mm512_xor_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_xor_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_xor_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_xor_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_xor_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_xor_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_xor_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_xor_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_64x64_x64_AVX512& x){
        vec[0] = _mm512_or_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_or_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_or_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_or_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_or_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_or_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_or_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_or_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_64x64_x64_AVX512& x){
        vec[0] = _mm512_and_si512(vec[0], x.vec[0]);
        vec[1] = _mm512_and_si512(vec[1], x.vec[1]);
        vec[2] = _mm512_and_si512(vec[2], x.vec[2]);
        vec[3] = _mm512_and_si512(vec[3], x.vec[3]);
        vec[4] = _mm512_and_si512(vec[4], x.vec[4]);
        vec[5] = _mm512_and_si512(vec[5], x.vec[5]);
        vec[6] = _mm512_and_si512(vec[6], x.vec[6]);
        vec[7] = _mm512_and_si512(vec[7], x.vec[7]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_64x64_x64_AVX512& x){
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
    PA_FORCE_INLINE uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t bottom() const{
        return ((const uint64_t*)vec)[63];
    }
    PA_FORCE_INLINE uint64_t& bottom(){
        return ((uint64_t*)vec)[63];
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
        for (size_t c = 0; c < 64; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }


public:
    //  Copy the current tile into "tile" while applying the specified shifts.
    //  These are used to implement submatrix extraction where the desired
    //  sub-matrix may of arbitrary shift and alignment.

    void copy_to_shift_pp(BinaryTile_64x64_x64_AVX512& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, +y)
        __m512i shift = _mm512_set1_epi64(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 57){
            __m512i r0 = _mm512_loadu_si512((const __m512i*)(src + shift_y));
            r0 = _mm512_srlv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)dest));
            _mm512_store_si512((__m512i*)dest, r0);
            dest += 8;
            shift_y += 8;
        }
        if (shift_y < 64){
            __mmask8 mask = _mm512_cmpgt_epu64_mask(
                _mm512_setr_epi64(64, 63, 62, 61, 60, 59, 58, 57),
                _mm512_set1_epi64(shift_y)
            );
            __m512i r0 = _mm512_maskz_load_epi64(mask, (const int64_t*)(src + shift_y));
            r0 = _mm512_srlv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m256i*)dest));
            _mm512_store_si512((__m256i*)dest, r0);
        }
    }
    void copy_to_shift_np(BinaryTile_64x64_x64_AVX512& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, +y)
        __m512i shift = _mm512_set1_epi64(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 57){
            __m512i r0 = _mm512_loadu_si512((const __m512i*)(src + shift_y));
            r0 = _mm512_sllv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)dest));
            _mm512_store_si512((__m512i*)dest, r0);
            dest += 8;
            shift_y += 8;
        }
        if (shift_y < 64){
            __mmask8 mask = _mm512_cmpgt_epu64_mask(
                _mm512_setr_epi64(64, 63, 62, 61, 60, 59, 58, 57),
                _mm512_set1_epi64(shift_y)
            );
            __m512i r0 = _mm512_maskz_load_epi64(mask, (const int64_t*)(src + shift_y));
            r0 = _mm512_sllv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m256i*)dest));
            _mm512_store_si512((__m256i*)dest, r0);
        }
    }
    void copy_to_shift_pn(BinaryTile_64x64_x64_AVX512& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, -y)
        __m512i shift = _mm512_set1_epi64(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        size_t align = (64 - shift_y) & 7;
        if (align){
            src += align - 8;
            shift_y += align - 8;
            __mmask8 mask = _mm512_cmpgt_epu64_mask(
                _mm512_set1_epi64(align),
                _mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0)
            );
            __m512i r0 = _mm512_maskz_load_epi64(mask, (const int64_t*)src);
            r0 = _mm512_srlv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)(dest + shift_y)));
            _mm512_store_si512((__m512i*)(dest + shift_y), r0);
            src += 8;
            shift_y += 8;
        }
        while (shift_y < 64){
            __m512i r0 = _mm512_loadu_si512((const __m512i*)src);
            r0 = _mm512_srlv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)(dest + shift_y)));
            _mm512_store_si512((__m512i*)(dest + shift_y), r0);
            src += 8;
            shift_y += 8;
        }
    }
    void copy_to_shift_nn(BinaryTile_64x64_x64_AVX512& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, -y)
        __m512i shift = _mm512_set1_epi64(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        size_t align = (64 - shift_y) & 7;
        if (align){
            src += align - 8;
            shift_y += align - 8;
            __mmask8 mask = _mm512_cmpgt_epu64_mask(
                _mm512_set1_epi64(align),
                _mm512_setr_epi64(7, 6, 5, 4, 3, 2, 1, 0)
            );
            __m512i r0 = _mm512_maskz_load_epi64(mask, (const int64_t*)src);
            r0 = _mm512_sllv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)(dest + shift_y)));
            _mm512_store_si512((__m512i*)(dest + shift_y), r0);
            src += 8;
            shift_y += 8;
        }
        while (shift_y < 64){
            __m512i r0 = _mm512_loadu_si512((const __m512i*)src);
            r0 = _mm512_sllv_epi64(r0, shift);
            r0 = _mm512_or_si512(r0, _mm512_load_si512((__m512i*)(dest + shift_y)));
            _mm512_store_si512((__m512i*)(dest + shift_y), r0);
            src += 8;
            shift_y += 8;
        }
    }
};





}
}
#endif
