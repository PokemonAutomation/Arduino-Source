/*  Waterfill Tile (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Kernels_Arch.h"
#ifdef PA_Arch_x64_AVX2

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels_Waterfill_Tile_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{



bool find_bit(size_t& x, size_t& y, const BinaryTile_AVX2& tile){
    __m256i anything = tile.vec[0];
    anything = _mm256_or_si256(anything, tile.vec[1]);
    anything = _mm256_or_si256(anything, tile.vec[2]);
    anything = _mm256_or_si256(anything, tile.vec[3]);
    if (_mm256_testz_si256(anything, anything)){
        return false;
    }
    for (size_t c = 0; c < 16; c++){
        size_t pos;
        if (trailing_zeros(pos, tile.row(c))){
            x = pos;
            y = c;
            return true;
        }
    }
    return false;
}



void boundaries(
    const BinaryTile_AVX2& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    __m256i all_or_v = tile.vec[0];
    all_or_v = _mm256_or_si256(all_or_v, tile.vec[1]);
    all_or_v = _mm256_or_si256(all_or_v, tile.vec[2]);
    all_or_v = _mm256_or_si256(all_or_v, tile.vec[3]);
    uint64_t all_or = reduce_or64_x64_AVX2(all_or_v);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

#ifdef BINARY_TILE_X64_AVX2_FLAT
    __m256i row = _mm256_setr_epi64x(0, 1, 2, 3);
#else
    __m256i row = _mm256_setr_epi64x(0, 4, 8, 12);
#endif
    __m256i min = _mm256_set1_epi64x(15);
    __m256i max = _mm256_setzero_si256();
    for (size_t c = 0; c < 4; c++){
        __m256i vec = tile.vec[c];
        __m256i mask = _mm256_cmpeq_epi64(vec, _mm256_setzero_si256());
        __m256i minr = _mm256_or_si256(row, _mm256_and_si256(mask, _mm256_set1_epi64x(15)));
        __m256i maxr = _mm256_andnot_si256(mask, row);
        min = _mm256_min_epi32(min, minr);
        max = _mm256_max_epi32(max, maxr);
#ifdef BINARY_TILE_X64_AVX2_FLAT
        row = _mm256_add_epi64(row, _mm256_set1_epi64x(4));
#else
        row = _mm256_sub_epi64(row, _mm256_set1_epi64x(-1));
#endif
    }
    {
        __m128i x = _mm_min_epi32(
            _mm256_castsi256_si128(min),
            _mm256_extracti128_si256(min, 1)
        );
        x = _mm_min_epi32(x, _mm_unpackhi_epi64(x, x));
        min_y = _mm_cvtsi128_si64(x);
    }
    {
        __m128i x = _mm_max_epi32(
            _mm256_castsi256_si128(max),
            _mm256_extracti128_si256(max, 1)
        );
        x = _mm_max_epi32(x, _mm_unpackhi_epi64(x, x));
        max_y = _mm_cvtsi128_si64(x) + 1;
    }
}



__m256i popcount_indexsum(__m256i& sum_index, __m256i x){
    //  1 -> 2
    __m256i sum_high;
    __m256i pop_high = _mm256_and_si256(_mm256_srli_epi16(x, 1), _mm256_set1_epi8(0x55));
    __m256i sumxaxis = pop_high;
    __m256i popcount = _mm256_add_epi8(_mm256_and_si256(x, _mm256_set1_epi8(0x55)), pop_high);

    //  2 -> 4
    sum_high = _mm256_and_si256(_mm256_srli_epi16(sumxaxis, 2), _mm256_set1_epi8(0x33));
    pop_high = _mm256_and_si256(_mm256_srli_epi16(popcount, 2), _mm256_set1_epi8(0x33));
    sumxaxis = _mm256_add_epi8(_mm256_and_si256(sumxaxis, _mm256_set1_epi8(0x33)), sum_high);
    sumxaxis = _mm256_add_epi8(sumxaxis, _mm256_slli_epi16(pop_high, 1));
    popcount = _mm256_add_epi8(_mm256_and_si256(popcount, _mm256_set1_epi8(0x33)), pop_high);

    //  4 -> 8
    sum_high = _mm256_and_si256(_mm256_srli_epi16(sumxaxis, 4), _mm256_set1_epi8(0x0f));
    pop_high = _mm256_and_si256(_mm256_srli_epi16(popcount, 4), _mm256_set1_epi8(0x0f));
    sumxaxis = _mm256_add_epi8(_mm256_and_si256(sumxaxis, _mm256_set1_epi8(0x0f)), sum_high);
    sumxaxis = _mm256_add_epi8(sumxaxis, _mm256_slli_epi16(pop_high, 2));
    popcount = _mm256_add_epi8(_mm256_and_si256(popcount, _mm256_set1_epi8(0x0f)), pop_high);

    //  8 -> 16
    sum_high = _mm256_srli_epi16(sumxaxis, 8);
    pop_high = _mm256_srli_epi16(popcount, 8);
    sumxaxis = _mm256_add_epi16(_mm256_and_si256(sumxaxis, _mm256_set1_epi16(0x00ff)), sum_high);
    sumxaxis = _mm256_add_epi16(sumxaxis, _mm256_slli_epi16(pop_high, 3));
    popcount = _mm256_add_epi16(_mm256_and_si256(popcount, _mm256_set1_epi16(0x00ff)), pop_high);

    //  16 -> 32
    sum_high = _mm256_srli_epi32(sumxaxis, 16);
    pop_high = _mm256_srli_epi32(popcount, 16);
    sumxaxis = _mm256_add_epi32(sumxaxis, sum_high);
    sumxaxis = _mm256_add_epi32(sumxaxis, _mm256_slli_epi32(pop_high, 4));
    popcount = _mm256_add_epi32(popcount, pop_high);

    //  32 -> 64
    sum_high = _mm256_srli_epi64(sumxaxis, 32);
    pop_high = _mm256_srli_epi64(popcount, 32);
    sumxaxis = _mm256_add_epi64(sumxaxis, sum_high);
    sumxaxis = _mm256_add_epi64(sumxaxis, _mm256_slli_epi64(pop_high, 5));
    popcount = _mm256_add_epi64(popcount, pop_high);

    sum_index = _mm256_and_si256(sumxaxis, _mm256_set1_epi64x(0x000000000000ffff));
    return _mm256_and_si256(popcount, _mm256_set1_epi64x(0x000000000000ffff));
}
uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_AVX2& tile
){
    __m256i sum_p, sum_x, sum_y;
    {
        __m256i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[0]);
        sum_p = pop;
        sum_x = sum;
#ifdef BINARY_TILE_X64_AVX2_FLAT
        sum_y = _mm256_mul_epu32(pop, _mm256_setr_epi64x(0, 1, 2, 3));
#else
        sum_y = _mm256_mul_epu32(pop, _mm256_setr_epi64x(0, 4, 8, 12));
#endif
    }
    {
        __m256i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[1]);
        sum_p = _mm256_add_epi64(sum_p, pop);
        sum_x = _mm256_add_epi64(sum_x, sum);
#ifdef BINARY_TILE_X64_AVX2_FLAT
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(4, 5, 6, 7)));
#else
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(1, 5, 9, 13)));
#endif
    }
    {
        __m256i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[2]);
        sum_p = _mm256_add_epi64(sum_p, pop);
        sum_x = _mm256_add_epi64(sum_x, sum);
#ifdef BINARY_TILE_X64_AVX2_FLAT
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(8, 9, 10, 11)));
#else
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(2, 6, 10, 14)));
#endif
    }
    {
        __m256i pop, sum;
        pop = popcount_indexsum(sum, tile.vec[3]);
        sum_p = _mm256_add_epi64(sum_p, pop);
        sum_x = _mm256_add_epi64(sum_x, sum);
#ifdef BINARY_TILE_X64_AVX2_FLAT
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(12, 13, 14, 15)));
#else
        sum_y = _mm256_add_epi64(sum_y, _mm256_mul_epu32(pop, _mm256_setr_epi64x(3, 7, 11, 15)));
#endif
    }
    sum_xcoord = reduce_add64_x64_AVX2(sum_x);
    sum_ycoord = reduce_add64_x64_AVX2(sum_y);
    return reduce_add64_x64_AVX2(sum_p);
}




PA_FORCE_INLINE __m256i bit_reverse(__m256i x){
    __m256i r0, r1;

    x = _mm256_shuffle_epi8(
        x,
        _mm256_setr_epi8(
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8,
            7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8
        )
    );

    r0 = _mm256_srli_epi32(x, 4);
    r1 = _mm256_slli_epi32(x, 4);
    r0 = _mm256_and_si256(r0, _mm256_set1_epi8((uint8_t)0x0f));
    r1 = _mm256_and_si256(r1, _mm256_set1_epi8((uint8_t)0xf0));
    r1 = _mm256_or_si256(r0, r1);

    r0 = _mm256_srli_epi32(r1, 2);
    r1 = _mm256_slli_epi32(r1, 2);
    r0 = _mm256_and_si256(r0, _mm256_set1_epi8((uint8_t)0x33));
    r1 = _mm256_and_si256(r1, _mm256_set1_epi8((uint8_t)0xcc));
    r1 = _mm256_or_si256(r0, r1);

    r0 = _mm256_srli_epi32(r1, 1);
    r1 = _mm256_slli_epi32(r1, 1);
    r0 = _mm256_and_si256(r0, _mm256_set1_epi8((uint8_t)0x55));
    r1 = _mm256_and_si256(r1, _mm256_set1_epi8((uint8_t)0xaa));
    r1 = _mm256_or_si256(r0, r1);

    return r1;
}



#ifdef BINARY_TILE_X64_AVX2_FLAT
struct ProcessedMask{
    __m256i m0, m1, m2, m3; //  Copy of the masks.
    __m256i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m256i t0, t1, t2, t3; //  Transposed masks.
    __m256i f1, f2, f3;     //  Forward-carry mask.
    __m256i r0, r1, r2;     //  Reverse-carry mask.

    PA_FORCE_INLINE ProcessedMask(
        const BinaryTile_AVX2& m,
        __m256i x0, __m256i x1, __m256i x2, __m256i x3
    ){
        m0 = _mm256_or_si256(x0, m.vec[0]);
        m1 = _mm256_or_si256(x1, m.vec[1]);
        m2 = _mm256_or_si256(x2, m.vec[2]);
        m3 = _mm256_or_si256(x3, m.vec[3]);

        b0 = bit_reverse(m0);
        b1 = bit_reverse(m1);
        b2 = bit_reverse(m2);
        b3 = bit_reverse(m3);

        t0 = m0;
        t1 = m1;
        t2 = m2;
        t3 = m3;
        transpose_i64_4x4_AVX2(t0, t1, t2, t3);

        //  Forward carry
        __m256i f0 = t0;
        f1 = _mm256_and_si256(f0, t1);
        f2 = _mm256_and_si256(f1, t2);
        f3 = _mm256_and_si256(f2, t3);
        transpose_i64_4x4_AVX2(f0, f1, f2, f3);

        //  Reverse carry
        __m256i r3 = t3;
        r2 = _mm256_and_si256(r3, t2);
        r1 = _mm256_and_si256(r2, t1);
        r0 = _mm256_and_si256(r1, t0);
        transpose_i64_4x4_AVX2(r0, r1, r2, r3);
    }
};
#else
struct ProcessedMask{
    __m256i m0, m1, m2, m3; //  Copy of the masks.
    __m256i b0, b1, b2, b3; //  Bit-reversed copy of the masks.
    __m256i f1, f2, f3;     //  Forward-carry mask.
    __m256i r0, r1, r2;     //  Reverse-carry mask.

    PA_FORCE_INLINE ProcessedMask(
        const BinaryTile_AVX2& m,
        __m256i x0, __m256i x1, __m256i x2, __m256i x3
    ){
        m0 = _mm256_or_si256(x0, m.vec[0]);
        m1 = _mm256_or_si256(x1, m.vec[1]);
        m2 = _mm256_or_si256(x2, m.vec[2]);
        m3 = _mm256_or_si256(x3, m.vec[3]);

        b0 = bit_reverse(m0);
        b1 = bit_reverse(m1);
        b2 = bit_reverse(m2);
        b3 = bit_reverse(m3);

        //  Forward carry
        __m256i f0 = m0;
        f1 = _mm256_and_si256(f0, m1);
        f2 = _mm256_and_si256(f1, m2);
        f3 = _mm256_and_si256(f2, m3);
        transpose_i64_4x4_AVX2(f0, f1, f2, f3);

        //  Reverse carry
        __m256i r3 = m3;
        r2 = _mm256_and_si256(r3, m2);
        r1 = _mm256_and_si256(r2, m1);
        r0 = _mm256_and_si256(r1, m0);
        transpose_i64_4x4_AVX2(r0, r1, r2, r3);
    }
};
#endif


PA_FORCE_INLINE void expand_reverse(__m256i m, __m256i b, __m256i& x){
    __m256i s = bit_reverse(_mm256_add_epi64(bit_reverse(x), b));
    s = _mm256_andnot_si256(s, m);
    x = _mm256_or_si256(x, s);
}

PA_FORCE_INLINE void expand_forward(
    const ProcessedMask& mask,
    __m256i& x0, __m256i& x1, __m256i& x2, __m256i& x3
){
    __m256i s0 = _mm256_add_epi64(x0, mask.m0);
    __m256i s1 = _mm256_add_epi64(x1, mask.m1);
    __m256i s2 = _mm256_add_epi64(x2, mask.m2);
    __m256i s3 = _mm256_add_epi64(x3, mask.m3);

    s0 = _mm256_andnot_si256(s0, mask.m0);
    s1 = _mm256_andnot_si256(s1, mask.m1);
    s2 = _mm256_andnot_si256(s2, mask.m2);
    s3 = _mm256_andnot_si256(s3, mask.m3);

    x0 = _mm256_or_si256(x0, s0);
    x1 = _mm256_or_si256(x1, s1);
    x2 = _mm256_or_si256(x2, s2);
    x3 = _mm256_or_si256(x3, s3);
}
PA_FORCE_INLINE void expand_reverse(
    const ProcessedMask& mask,
    __m256i& x0, __m256i& x1, __m256i& x2, __m256i& x3
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
}




PA_FORCE_INLINE void expand_vertical(
    const ProcessedMask& mask,
    __m256i& x0, __m256i& x1, __m256i& x2, __m256i& x3
){
#ifdef BINARY_TILE_X64_AVX2_FLAT
    //  Carry across adjacent rows.
    transpose_i64_4x4_AVX2(x0, x1, x2, x3);
    x1 = _mm256_or_si256(x1, _mm256_and_si256(x0, mask.t1));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(x3, mask.t2));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(x1, mask.t2));
    x1 = _mm256_or_si256(x1, _mm256_and_si256(x2, mask.t1));
    x3 = _mm256_or_si256(x3, _mm256_and_si256(x2, mask.t3));
    x0 = _mm256_or_si256(x0, _mm256_and_si256(x1, mask.t0));
    transpose_i64_4x4_AVX2(x0, x1, x2, x3);

    //  Carry across groups of 4 rows.
    x1 = _mm256_or_si256(x1, _mm256_and_si256(_mm256_permute4x64_epi64(x0, 255), mask.f1));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(_mm256_permute4x64_epi64(x3,   0), mask.r2));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(_mm256_permute4x64_epi64(x1, 255), mask.f2));
    x1 = _mm256_or_si256(x1, _mm256_and_si256(_mm256_permute4x64_epi64(x2,   0), mask.r1));
    x3 = _mm256_or_si256(x3, _mm256_and_si256(_mm256_permute4x64_epi64(x2, 255), mask.f3));
    x0 = _mm256_or_si256(x0, _mm256_and_si256(_mm256_permute4x64_epi64(x1,   0), mask.r0));
#else
    //  Carry across adjacent rows.
    x1 = _mm256_or_si256(x1, _mm256_and_si256(x0, mask.m1));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(x3, mask.m2));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(x1, mask.m2));
    x1 = _mm256_or_si256(x1, _mm256_and_si256(x2, mask.m1));
    x3 = _mm256_or_si256(x3, _mm256_and_si256(x2, mask.m3));
    x0 = _mm256_or_si256(x0, _mm256_and_si256(x1, mask.m0));

    //  Carry across groups of 4 rows.
    transpose_i64_4x4_AVX2(x0, x1, x2, x3);
    x1 = _mm256_or_si256(x1, _mm256_and_si256(_mm256_permute4x64_epi64(x0, 255), mask.f1));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(_mm256_permute4x64_epi64(x3,   0), mask.r2));
    x2 = _mm256_or_si256(x2, _mm256_and_si256(_mm256_permute4x64_epi64(x1, 255), mask.f2));
    x1 = _mm256_or_si256(x1, _mm256_and_si256(_mm256_permute4x64_epi64(x2,   0), mask.r1));
    x3 = _mm256_or_si256(x3, _mm256_and_si256(_mm256_permute4x64_epi64(x2, 255), mask.f3));
    x0 = _mm256_or_si256(x0, _mm256_and_si256(_mm256_permute4x64_epi64(x1,   0), mask.r0));
    transpose_i64_4x4_AVX2(x0, x1, x2, x3);
#endif
}

void waterfill_expand(const BinaryTile_AVX2& m, BinaryTile_AVX2& x){
    __m256i x0 = x.vec[0];
    __m256i x1 = x.vec[1];
    __m256i x2 = x.vec[2];
    __m256i x3 = x.vec[3];

    ProcessedMask mask(m, x0, x1, x2, x3);

    __m256i changed;
    do{
        expand_forward(mask, x0, x1, x2, x3);
        expand_vertical(mask, x0, x1, x2, x3);
        expand_reverse(mask, x0, x1, x2, x3);
        changed = _mm256_xor_si256(x0, x.vec[0]);
        x.vec[0] = x0;
        changed = _mm256_or_si256(changed, _mm256_xor_si256(x1, x.vec[1]));
        x.vec[1] = x1;
        changed = _mm256_or_si256(changed, _mm256_xor_si256(x2, x.vec[2]));
        x.vec[2] = x2;
        changed = _mm256_or_si256(changed, _mm256_xor_si256(x3, x.vec[3]));
        x.vec[3] = x3;
//        cout << x.dump() << endl;
    }while (!_mm256_testz_si256(changed, changed));
}



bool waterfill_touch_top(const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border){
    uint64_t available = mask.top() & ~tile.top();
    uint64_t new_bits = available & border.bottom();
    if (new_bits == 0){
        return false;
    }
    tile.top() |= new_bits;
    return true;
}
bool waterfill_touch_bottom(const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border){
    uint64_t available = mask.bottom() & ~tile.bottom();
    uint64_t new_bits = available & border.top();
    if (new_bits == 0){
        return false;
    }
    tile.bottom() |= new_bits;
    return true;
}
bool waterfill_touch_left(const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border){
    __m256i changed = _mm256_setzero_si256();
    for (size_t c = 0; c < 4; c++){
        __m256i available = _mm256_andnot_si256(tile.vec[c], mask.vec[c]);
        __m256i new_bits = _mm256_and_si256(available, _mm256_srli_epi64(border.vec[c], 63));
        changed = _mm256_or_si256(changed, new_bits);
        tile.vec[c] = _mm256_or_si256(tile.vec[c], new_bits);
    }
    return !_mm256_testz_si256(changed, changed);
}
bool waterfill_touch_right(const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border){
    __m256i changed = _mm256_setzero_si256();
    for (size_t c = 0; c < 4; c++){
        __m256i available = _mm256_andnot_si256(tile.vec[c], mask.vec[c]);
        __m256i new_bits = _mm256_and_si256(available, _mm256_slli_epi64(border.vec[c], 63));
        changed = _mm256_or_si256(changed, new_bits);
        tile.vec[c] = _mm256_or_si256(tile.vec[c], new_bits);
    }
    return !_mm256_testz_si256(changed, changed);
}









}
}
}
#endif
