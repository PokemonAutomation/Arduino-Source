/*  Waterfill Tile (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels/Kernels_Arch.h"
#ifdef PA_Arch_x64_AVX2

#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels_Waterfill_Tile_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{


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





}
}
#endif
