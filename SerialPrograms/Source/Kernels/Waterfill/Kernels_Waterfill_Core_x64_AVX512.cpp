/*  Waterfill Core (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_17_Skylake

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_Waterfill_Routines.h"
#include "Kernels_Waterfill_Core_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



bool Waterfill_x64_AVX512::find_bit(size_t& x, size_t& y, const BinaryTile_AVX512& tile){
    __m512i anything = tile.vec[0];
    anything = _mm512_or_si512(anything, tile.vec[1]);
    anything = _mm512_or_si512(anything, tile.vec[2]);
    anything = _mm512_or_si512(anything, tile.vec[3]);
    anything = _mm512_or_si512(anything, tile.vec[4]);
    anything = _mm512_or_si512(anything, tile.vec[5]);
    anything = _mm512_or_si512(anything, tile.vec[6]);
    anything = _mm512_or_si512(anything, tile.vec[7]);
    if (!_mm512_test_epi64_mask(anything, anything)){
        return false;
    }
    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    for (size_t c = 0; c < 8; c++){
        __m512i vec = tile.vec[c];
        __mmask8 mask = _mm512_test_epi64_mask(vec, vec);
        if (mask){
            vec = _mm512_maskz_compress_epi64(mask, vec);
            row = _mm512_maskz_compress_epi64(mask, row);
            uint64_t word = _mm_cvtsi128_si64(_mm512_castsi512_si128(vec));
            y = _mm_cvtsi128_si64(_mm512_castsi512_si128(row));
            trailing_zeros(x, word);
            return true;
        }
        row = _mm512_add_epi64(row, _mm512_set1_epi64(8));
    }
    return false;
}



void Waterfill_x64_AVX512::boundaries(
    const BinaryTile_AVX512& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    __m512i all_or_v = tile.vec[0];
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[1]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[2]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[3]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[4]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[5]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[6]);
    all_or_v = _mm512_or_si512(all_or_v, tile.vec[7]);
    uint64_t all_or = _mm512_reduce_or_epi64(all_or_v);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    __m512i row = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i min = _mm512_set1_epi64(-1);
    __m512i max = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
        __m512i vec = tile.vec[c];
        __mmask8 mask = _mm512_cmpneq_epu64_mask(vec, _mm512_setzero_si512());
        min = _mm512_mask_min_epu64(min, mask, min, row);
        max = _mm512_mask_max_epu64(max, mask, max, row);
        row = _mm512_add_epi64(row, _mm512_set1_epi64(8));
    }
    min_y = _mm512_reduce_min_epu64(min);
    max_y = _mm512_reduce_max_epu64(max) + 1;
}



__m512i Waterfill_x64_AVX512::popcount_indexsum(__m512i& sum_index, __m512i x){
    //  1 -> 2
    __m512i sum_high;
    __m512i pop_high = _mm512_and_si512(_mm512_srli_epi16(x, 1), _mm512_set1_epi8(0x55));
    __m512i sumxaxis = pop_high;
    __m512i popcount = _mm512_add_epi8(_mm512_and_si512(x, _mm512_set1_epi8(0x55)), pop_high);

    //  2 -> 4
    sum_high = _mm512_and_si512(_mm512_srli_epi16(sumxaxis, 2), _mm512_set1_epi8(0x33));
    pop_high = _mm512_and_si512(_mm512_srli_epi16(popcount, 2), _mm512_set1_epi8(0x33));
    sumxaxis = _mm512_add_epi8(_mm512_and_si512(sumxaxis, _mm512_set1_epi8(0x33)), sum_high);
    sumxaxis = _mm512_add_epi8(sumxaxis, _mm512_slli_epi16(pop_high, 1));
    popcount = _mm512_add_epi8(_mm512_and_si512(popcount, _mm512_set1_epi8(0x33)), pop_high);

    //  4 -> 8
    sum_high = _mm512_and_si512(_mm512_srli_epi16(sumxaxis, 4), _mm512_set1_epi8(0x0f));
    pop_high = _mm512_and_si512(_mm512_srli_epi16(popcount, 4), _mm512_set1_epi8(0x0f));
    sumxaxis = _mm512_add_epi8(_mm512_and_si512(sumxaxis, _mm512_set1_epi8(0x0f)), sum_high);
    sumxaxis = _mm512_add_epi8(sumxaxis, _mm512_slli_epi16(pop_high, 2));
    popcount = _mm512_add_epi8(_mm512_and_si512(popcount, _mm512_set1_epi8(0x0f)), pop_high);

    //  8 -> 16
    sum_high = _mm512_srli_epi16(sumxaxis, 8);
    pop_high = _mm512_srli_epi16(popcount, 8);
    sumxaxis = _mm512_add_epi16(_mm512_and_si512(sumxaxis, _mm512_set1_epi16(0x00ff)), sum_high);
    sumxaxis = _mm512_add_epi16(sumxaxis, _mm512_slli_epi16(pop_high, 3));
    popcount = _mm512_add_epi16(_mm512_and_si512(popcount, _mm512_set1_epi16(0x00ff)), pop_high);

    //  16 -> 32
    sum_high = _mm512_srli_epi32(sumxaxis, 16);
    pop_high = _mm512_srli_epi32(popcount, 16);
    sumxaxis = _mm512_add_epi32(sumxaxis, sum_high);
    sumxaxis = _mm512_add_epi32(sumxaxis, _mm512_slli_epi32(pop_high, 4));
    popcount = _mm512_add_epi32(popcount, pop_high);

    //  32 -> 64
    sum_high = _mm512_srli_epi64(sumxaxis, 32);
    pop_high = _mm512_srli_epi64(popcount, 32);
    sumxaxis = _mm512_add_epi64(sumxaxis, sum_high);
    sumxaxis = _mm512_add_epi64(sumxaxis, _mm512_slli_epi64(pop_high, 5));
    popcount = _mm512_add_epi64(popcount, pop_high);

    sum_index = _mm512_and_si512(sumxaxis, _mm512_set1_epi64(0x000000000000ffff));
    return _mm512_and_si512(popcount, _mm512_set1_epi64(0x000000000000ffff));
}
uint64_t Waterfill_x64_AVX512::popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_AVX512& tile
){
    __m512i sum_p, sum_x, sum_y;
    __m512i offsets = _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    {
        __m512i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[0]);
        sum_p = pop;
        sum_x = sum;
        sum_y = _mm512_mul_epu32(pop, offsets);
    }
    for (size_t c = 1; c < 8; c++){
        __m512i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[c]);
        sum_p = _mm512_add_epi64(sum_p, pop);
        sum_x = _mm512_add_epi64(sum_x, sum);
        offsets = _mm512_add_epi64(offsets, _mm512_set1_epi64(8));
        sum_y = _mm512_add_epi64(sum_y, _mm512_mul_epu32(pop, offsets));
    }
    sum_xcoord = _mm512_reduce_add_epi64(sum_x);
    sum_ycoord = _mm512_reduce_add_epi64(sum_y);
    return _mm512_reduce_add_epi64(sum_p);
}


template <> PA_FORCE_INLINE __m512i bit_reverse<false>(__m512i x){
    x = _mm512_shuffle_epi8(
        x,
        _mm512_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );

    __m512i r0, r1;
    r0 = _mm512_srli_epi32(x, 4);
    r1 = _mm512_slli_epi32(x, 4);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x0f), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 2);
    r1 = _mm512_slli_epi32(r1, 2);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x33), 0b11011000);

    r0 = _mm512_srli_epi32(r1, 1);
    r1 = _mm512_slli_epi32(r1, 1);
    r1 = _mm512_ternarylogic_epi64(r1, r0, _mm512_set1_epi8((uint8_t)0x55), 0b11011000);

    return r1;
}




void Waterfill_x64_AVX512::Waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x){
    Waterfill::Waterfill_expand<false>(m, x);
}



bool Waterfill_x64_AVX512::Waterfill_touch_top(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border){
    uint64_t available = mask.top() & ~tile.top();
    uint64_t new_bits = available & border.bottom();
    if (new_bits == 0){
        return false;
    }
    tile.top() |= new_bits;
    return true;
}
bool Waterfill_x64_AVX512::Waterfill_touch_bottom(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border){
    uint64_t available = mask.bottom() & ~tile.bottom();
    uint64_t new_bits = available & border.top();
    if (new_bits == 0){
        return false;
    }
    tile.bottom() |= new_bits;
    return true;
}
bool Waterfill_x64_AVX512::Waterfill_touch_left(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
        __m512i available = _mm512_andnot_si512(tile.vec[c], mask.vec[c]);
        __m512i new_bits = _mm512_and_si512(available, _mm512_srli_epi64(border.vec[c], 63));
        changed = _mm512_or_si512(changed, new_bits);
        tile.vec[c] = _mm512_or_si512(tile.vec[c], new_bits);
    }
    return _mm512_test_epi64_mask(changed, changed);
}
bool Waterfill_x64_AVX512::Waterfill_touch_right(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border){
    __m512i changed = _mm512_setzero_si512();
    for (size_t c = 0; c < 8; c++){
        __m512i available = _mm512_andnot_si512(tile.vec[c], mask.vec[c]);
        __m512i new_bits = _mm512_and_si512(available, _mm512_slli_epi64(border.vec[c], 63));
        changed = _mm512_or_si512(changed, new_bits);
        tile.vec[c] = _mm512_or_si512(tile.vec[c], new_bits);
    }
    return _mm512_test_epi64_mask(changed, changed);
}





bool find_object_on_bit_x64_AVX512(PackedBinaryMatrix_IB& matrix, WaterfillObject& object, size_t x, size_t y){
    return find_object_on_bit<BinaryTile_AVX512, Waterfill_x64_AVX512>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        object, x, y
    );
}

std::vector<WaterfillObject> find_objects_inplace_x64_AVX512(PackedBinaryMatrix_IB& matrix, size_t min_area, bool keep_objects){
    return find_objects_inplace<BinaryTile_AVX512, Waterfill_x64_AVX512>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        min_area, keep_objects
    );
}
std::unique_ptr<WaterfillIterator2> make_WaterfillIterator_x64_AVX512(PackedBinaryMatrix_IB& matrix, size_t min_area){
    return std::make_unique<WaterfillIterator_TI<BinaryTile_AVX512, Waterfill_x64_AVX512>>(
        static_cast<PackedBinaryMatrix_x64_AVX512&>(matrix).get(),
        min_area
    );
}







}
}
}
#endif
