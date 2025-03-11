/*  Pixel Sum + Sum of Squares (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <smmintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
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



PA_FORCE_INLINE void pixel_sum_sqr_x64_SSE41(
    PixelSums& sums,
    uint16_t width,
    const uint32_t* image,
    const uint32_t* alpha
){
    __m128i sumB = _mm_setzero_si128();
    __m128i sumG = _mm_setzero_si128();
    __m128i sumR = _mm_setzero_si128();
    __m128i sumA = _mm_setzero_si128();
    __m128i sqrB = _mm_setzero_si128();
    __m128i sqrG = _mm_setzero_si128();
    __m128i sqrR = _mm_setzero_si128();

    const __m128i* ptrI = (const __m128i*)image;
    const __m128i* ptrA = (const __m128i*)alpha;

    size_t lc = width / 4;
    do{
        __m128i p = _mm_loadu_si128(ptrI);
        __m128i m = _mm_loadu_si128(ptrA);

        m = _mm_srai_epi32(m, 31);
        p = _mm_and_si128(p, m);

        __m128i r0 = _mm_and_si128(p, _mm_set1_epi32(0x000000ff));
//        __m128i r1 = _mm_and_si128(_mm_srli_epi32(p, 8), _mm_set1_epi32(0x000000ff));
//        __m128i r2 = _mm_and_si128(_mm_srli_epi32(p, 16), _mm_set1_epi32(0x000000ff));
        __m128i r1 = _mm_shuffle_epi8(p, _mm_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1));
        __m128i r2 = _mm_shuffle_epi8(p, _mm_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1));

        sumB = _mm_add_epi32(sumB, r0);
        sumG = _mm_add_epi32(sumG, r1);
        sumR = _mm_add_epi32(sumR, r2);
        sumA = _mm_sub_epi32(sumA, m);

        r0 = _mm_mullo_epi16(r0, r0);
        r1 = _mm_mullo_epi16(r1, r1);
        r2 = _mm_mullo_epi16(r2, r2);

        sqrB = _mm_add_epi32(sqrB, r0);
        sqrG = _mm_add_epi32(sqrG, r1);
        sqrR = _mm_add_epi32(sqrR, r2);

        ptrI++;
        ptrA++;
    }while (--lc);

    if (width % 4){
        PartialWordAccess_x64_SSE41 loader(width * sizeof(uint32_t) % 16);

        __m128i p = loader.load_int_no_read_past_end(ptrI);
        __m128i m = loader.load_int_no_read_past_end(ptrA);

        m = _mm_srai_epi32(m, 31);
        p = _mm_and_si128(p, m);

        __m128i r0 = _mm_and_si128(p, _mm_set1_epi32(0x000000ff));
//        __m128i r1 = _mm_and_si128(_mm_srli_epi32(p, 8), _mm_set1_epi32(0x000000ff));
//        __m128i r2 = _mm_and_si128(_mm_srli_epi32(p, 16), _mm_set1_epi32(0x000000ff));
        __m128i r1 = _mm_shuffle_epi8(p, _mm_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1));
        __m128i r2 = _mm_shuffle_epi8(p, _mm_setr_epi8(2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1));

        sumB = _mm_add_epi32(sumB, r0);
        sumG = _mm_add_epi32(sumG, r1);
        sumR = _mm_add_epi32(sumR, r2);
        sumA = _mm_sub_epi32(sumA, m);

        r0 = _mm_mullo_epi16(r0, r0);
        r1 = _mm_mullo_epi16(r1, r1);
        r2 = _mm_mullo_epi16(r2, r2);

        sqrB = _mm_add_epi32(sqrB, r0);
        sqrG = _mm_add_epi32(sqrG, r1);
        sqrR = _mm_add_epi32(sqrR, r2);
    }

    sums.count += reduce32_x64_SSE41(sumA);
    sums.sumR += reduce32_x64_SSE41(sumR);
    sums.sumG += reduce32_x64_SSE41(sumG);
    sums.sumB += reduce32_x64_SSE41(sumB);
    sums.sqrR += reduce32_x64_SSE41(sqrR);
    sums.sqrG += reduce32_x64_SSE41(sqrG);
    sums.sqrB += reduce32_x64_SSE41(sqrB);
}
void pixel_sum_sqr_x64_SSE41(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
    if (width < 4){
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
        pixel_sum_sqr_x64_SSE41(sums, (uint16_t)width, image, alpha);
        image = (const uint32_t*)((const char*)image + image_bytes_per_row);
        alpha = (const uint32_t*)((const char*)alpha + alpha_bytes_per_row);
    }
}



}
}
#endif
