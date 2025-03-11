/*  Pixel Sum + Sum of Squares (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_ImagePixelSumSqr.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_Default(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



PA_FORCE_INLINE void pixel_sum_sqr_x64_AVX512(
    PixelSums& sums,
    uint16_t width,
    const uint32_t* image,
    const uint32_t* alpha
){
    __m512i sumB = _mm512_setzero_si512();
    __m512i sumG = _mm512_setzero_si512();
    __m512i sumR = _mm512_setzero_si512();
    __m512i sumA = _mm512_setzero_si512();
    __m512i sqrB = _mm512_setzero_si512();
    __m512i sqrG = _mm512_setzero_si512();
    __m512i sqrR = _mm512_setzero_si512();

    const __m512i* ptrI = (const __m512i*)image;
    const __m512i* ptrA = (const __m512i*)alpha;

    size_t lc = width / 16;
    do{
        __m512i p = _mm512_loadu_si512(ptrI);
        __m512i m = _mm512_loadu_si512(ptrA);

        m = _mm512_srai_epi32(m, 31);
        p = _mm512_and_si512(p, m);

        __m512i r0 = _mm512_and_si512(p, _mm512_set1_epi32(0x000000ff));
//        __m512i r1 = _mm512_and_si512(_mm512_srli_epi32(p, 8), _mm512_set1_epi32(0x000000ff));
//        __m512i r2 = _mm512_and_si512(_mm512_srli_epi32(p, 16), _mm512_set1_epi32(0x000000ff));
        __m512i r1 = _mm512_shuffle_epi8(p, _mm512_setr_epi8(
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
        ));
        __m512i r2 = _mm512_shuffle_epi8(p, _mm512_setr_epi8(
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1
        ));

        sumB = _mm512_add_epi32(sumB, r0);
        sumG = _mm512_add_epi32(sumG, r1);
        sumR = _mm512_add_epi32(sumR, r2);
        sumA = _mm512_sub_epi32(sumA, m);

        r0 = _mm512_mullo_epi16(r0, r0);
        r1 = _mm512_mullo_epi16(r1, r1);
        r2 = _mm512_mullo_epi16(r2, r2);

        sqrB = _mm512_add_epi32(sqrB, r0);
        sqrG = _mm512_add_epi32(sqrG, r1);
        sqrR = _mm512_add_epi32(sqrR, r2);

        ptrI++;
        ptrA++;
    }while (--lc);

    if (width % 16){
        __mmask16 mask = (__mmask16)(((uint32_t)1 << (width % 16)) - 1);
        __m512i p = _mm512_maskz_loadu_epi32(mask, ptrI);
        __m512i m = _mm512_maskz_loadu_epi32(mask, ptrA);

        m = _mm512_srai_epi32(m, 31);
        p = _mm512_and_si512(p, m);

        __m512i r0 = _mm512_and_si512(p, _mm512_set1_epi32(0x000000ff));
//        __m512i r1 = _mm512_and_si512(_mm512_srli_epi32(p, 8), _mm512_set1_epi32(0x000000ff));
//        __m512i r2 = _mm512_and_si512(_mm512_srli_epi32(p, 16), _mm512_set1_epi32(0x000000ff));
        __m512i r1 = _mm512_shuffle_epi8(p, _mm512_setr_epi8(
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
        ));
        __m512i r2 = _mm512_shuffle_epi8(p, _mm512_setr_epi8(
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1
        ));

        sumB = _mm512_add_epi32(sumB, r0);
        sumG = _mm512_add_epi32(sumG, r1);
        sumR = _mm512_add_epi32(sumR, r2);
        sumA = _mm512_sub_epi32(sumA, m);

        r0 = _mm512_mullo_epi16(r0, r0);
        r1 = _mm512_mullo_epi16(r1, r1);
        r2 = _mm512_mullo_epi16(r2, r2);

        sqrB = _mm512_add_epi32(sqrB, r0);
        sqrG = _mm512_add_epi32(sqrG, r1);
        sqrR = _mm512_add_epi32(sqrR, r2);
    }

    sums.count += _mm512_reduce_add_epi32(sumA);
    sums.sumR += _mm512_reduce_add_epi32(sumR);
    sums.sumG += _mm512_reduce_add_epi32(sumG);
    sums.sumB += _mm512_reduce_add_epi32(sumB);
    sums.sqrR += _mm512_reduce_add_epi32(sqrR);
    sums.sqrG += _mm512_reduce_add_epi32(sqrG);
    sums.sqrB += _mm512_reduce_add_epi32(sqrB);
}
void pixel_sum_sqr_x64_AVX512(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
    if (width < 16){
        pixel_sum_sqr_Default(
            sums,
            width, height,
            image, image_bytes_per_row,
            alpha, alpha_bytes_per_row
        );
        return;
    }
    if (width > 65535){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width limit exceeded: " + std::to_string(width));
    }
    for (size_t r = 0; r < height; r++){
        pixel_sum_sqr_x64_AVX512(sums, (uint16_t)width, image, alpha);
        image = (const uint32_t*)((const char*)image + image_bytes_per_row);
        alpha = (const uint32_t*)((const char*)alpha + alpha_bytes_per_row);
    }
}



}
}
#endif
