/*  Binary Matrix Tile (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_64x8_x64_SSE42_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_64x8_x64_SSE42_H

#include <nmmintrin.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"
#include "Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_64x8_x64_SSE42{
    static constexpr BinaryMatrixType TYPE = BinaryMatrixType::i64x8_x64_SSE42;
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 8;

    __m128i vec[4];

public:
    PA_FORCE_INLINE BinaryTile_64x8_x64_SSE42(){
        set_zero();
    }
    PA_FORCE_INLINE BinaryTile_64x8_x64_SSE42(const BinaryTile_64x8_x64_SSE42& x){
        vec[0] = x.vec[0];
        vec[1] = x.vec[1];
        vec[2] = x.vec[2];
        vec[3] = x.vec[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_64x8_x64_SSE42& x){
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
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(1, 0));
        vec[0] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(3, 2));
        vec[1] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(5, 4));
        vec[2] = _mm_and_si128(mask, word);
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(7, 6));
        vec[3] = _mm_and_si128(mask, word);
    }
    PA_FORCE_INLINE void clear_padding(size_t width, size_t height){
        __m128i word = _mm_set1_epi64x(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        __m128i vheight = _mm_set1_epi64x(height);
        __m128i mask;
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(1, 0));
        vec[0] = _mm_and_si128(vec[0], _mm_and_si128(mask, word));
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(3, 2));
        vec[1] = _mm_and_si128(vec[1], _mm_and_si128(mask, word));
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(5, 4));
        vec[2] = _mm_and_si128(vec[2], _mm_and_si128(mask, word));
        mask = _mm_cmpgt_epi64(vheight, _mm_set_epi64x(7, 6));
        vec[3] = _mm_and_si128(vec[3], _mm_and_si128(mask, word));
    }
    PA_FORCE_INLINE void invert(){
        vec[0] = _mm_xor_si128(vec[0], _mm_set1_epi32(0xffffffff));
        vec[1] = _mm_xor_si128(vec[1], _mm_set1_epi32(0xffffffff));
        vec[2] = _mm_xor_si128(vec[2], _mm_set1_epi32(0xffffffff));
        vec[3] = _mm_xor_si128(vec[3], _mm_set1_epi32(0xffffffff));
    }
    PA_FORCE_INLINE void operator^=(const BinaryTile_64x8_x64_SSE42& x){
        vec[0] = _mm_xor_si128(vec[0], x.vec[0]);
        vec[1] = _mm_xor_si128(vec[1], x.vec[1]);
        vec[2] = _mm_xor_si128(vec[2], x.vec[2]);
        vec[3] = _mm_xor_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator|=(const BinaryTile_64x8_x64_SSE42& x){
        vec[0] = _mm_or_si128(vec[0], x.vec[0]);
        vec[1] = _mm_or_si128(vec[1], x.vec[1]);
        vec[2] = _mm_or_si128(vec[2], x.vec[2]);
        vec[3] = _mm_or_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void operator&=(const BinaryTile_64x8_x64_SSE42& x){
        vec[0] = _mm_and_si128(vec[0], x.vec[0]);
        vec[1] = _mm_and_si128(vec[1], x.vec[1]);
        vec[2] = _mm_and_si128(vec[2], x.vec[2]);
        vec[3] = _mm_and_si128(vec[3], x.vec[3]);
    }
    PA_FORCE_INLINE void andnot(const BinaryTile_64x8_x64_SSE42& x){
        vec[0] = _mm_andnot_si128(x.vec[0], vec[0]);
        vec[1] = _mm_andnot_si128(x.vec[1], vec[1]);
        vec[2] = _mm_andnot_si128(x.vec[2], vec[2]);
        vec[3] = _mm_andnot_si128(x.vec[3], vec[3]);
    }


public:
    PA_FORCE_INLINE uint64_t top() const{
        return ((const uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t& top(){
        return ((uint64_t*)vec)[0];
    }
    PA_FORCE_INLINE uint64_t bottom() const{
        return ((const uint64_t*)vec)[7];
    }
    PA_FORCE_INLINE uint64_t& bottom(){
        return ((uint64_t*)vec)[7];
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
        for (size_t c = 0; c < 8; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }


public:
    //  Copy the current tile into "tile" while applying the specified shifts.
    //  These are used to implement submatrix extraction where the desired
    //  sub-matrix may of arbitrary shift and alignment.

    void copy_to_shift_pp(BinaryTile_64x8_x64_SSE42& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, +y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 7){
            __m128i r0 = _mm_loadu_si128((const __m128i*)(src + shift_y));
            r0 = _mm_srl_epi64(r0, shift);
            r0 = _mm_or_si128(r0, _mm_load_si128((__m128i*)dest));
            _mm_store_si128((__m128i*)dest, r0);
            dest += 2;
            shift_y += 2;
        }
        if (shift_y < 8){
            dest[0] |= src[shift_y] >> shift_x;
        }
    }
    void copy_to_shift_np(BinaryTile_64x8_x64_SSE42& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, +y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        while (shift_y < 7){
            __m128i r0 = _mm_loadu_si128((const __m128i*)(src + shift_y));
            r0 = _mm_sll_epi64(r0, shift);
            r0 = _mm_or_si128(r0, _mm_load_si128((__m128i*)dest));
            _mm_store_si128((__m128i*)dest, r0);
            dest += 2;
            shift_y += 2;
        }
        if (shift_y < 8){
            dest[0] |= src[shift_y] << shift_x;
        }
    }
    void copy_to_shift_pn(BinaryTile_64x8_x64_SSE42& tile, size_t shift_x, size_t shift_y) const{
        //  (+x, -y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        if (shift_y & 1){
            dest[shift_y] |= src[0] >> shift_x;
            src++;
            shift_y++;
        }
        while (shift_y < 8){
            __m128i r0 = _mm_loadu_si128((const __m128i*)src);
            r0 = _mm_srl_epi64(r0, shift);
            r0 = _mm_or_si128(r0, _mm_load_si128((__m128i*)(dest + shift_y)));
            _mm_store_si128((__m128i*)(dest + shift_y), r0);
            src += 2;
            shift_y += 2;
        }
    }
    void copy_to_shift_nn(BinaryTile_64x8_x64_SSE42& tile, size_t shift_x, size_t shift_y) const{
        //  (-x, -y)
        __m128i shift = _mm_set1_epi64x(shift_x);
        const uint64_t* src = (const uint64_t*)vec;
        uint64_t* dest = (uint64_t*)tile.vec;
        if (shift_y & 1){
            dest[shift_y] |= src[0] << shift_x;
            src++;
            shift_y++;
        }
        while (shift_y < 8){
            __m128i r0 = _mm_loadu_si128((const __m128i*)src);
            r0 = _mm_sll_epi64(r0, shift);
            r0 = _mm_or_si128(r0, _mm_load_si128((__m128i*)(dest + shift_y)));
            _mm_store_si128((__m128i*)(dest + shift_y), r0);
            src += 2;
            shift_y += 2;
        }
    }
};





}
}
#endif
