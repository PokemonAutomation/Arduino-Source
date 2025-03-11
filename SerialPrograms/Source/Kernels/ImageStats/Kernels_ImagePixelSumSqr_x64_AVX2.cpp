/*  Pixel Sum + Sum of Squares (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <smmintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
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



PA_FORCE_INLINE void pixel_sum_sqr_x64_AVX2(
    PixelSums& sums,
    uint16_t width,
    const uint32_t* image,
    const uint32_t* alpha
){
    __m256i sumB = _mm256_setzero_si256();
    __m256i sumG = _mm256_setzero_si256();
    __m256i sumR = _mm256_setzero_si256();
    __m256i sumA = _mm256_setzero_si256();
    __m256i sqrB = _mm256_setzero_si256();
    __m256i sqrG = _mm256_setzero_si256();
    __m256i sqrR = _mm256_setzero_si256();

    const __m256i* ptrI = (const __m256i*)image;
    const __m256i* ptrA = (const __m256i*)alpha;

    size_t lc = width / 8;
    do{
        __m256i p = _mm256_loadu_si256(ptrI);
        __m256i m = _mm256_loadu_si256(ptrA);

        m = _mm256_srai_epi32(m, 31);
        p = _mm256_and_si256(p, m);

        __m256i r0 = _mm256_and_si256(p, _mm256_set1_epi32(0x000000ff));
//        __m256i r1 = _mm256_and_si256(_mm256_srli_epi32(p, 8), _mm256_set1_epi32(0x000000ff));
//        __m256i r2 = _mm256_and_si256(_mm256_srli_epi32(p, 16), _mm256_set1_epi32(0x000000ff));
        __m256i r1 = _mm256_shuffle_epi8(p, _mm256_setr_epi8(
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
        ));
        __m256i r2 = _mm256_shuffle_epi8(p, _mm256_setr_epi8(
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1
        ));

        sumB = _mm256_add_epi32(sumB, r0);
        sumG = _mm256_add_epi32(sumG, r1);
        sumR = _mm256_add_epi32(sumR, r2);
        sumA = _mm256_sub_epi32(sumA, m);

        r0 = _mm256_mullo_epi16(r0, r0);
        r1 = _mm256_mullo_epi16(r1, r1);
        r2 = _mm256_mullo_epi16(r2, r2);

        sqrB = _mm256_add_epi32(sqrB, r0);
        sqrG = _mm256_add_epi32(sqrG, r1);
        sqrR = _mm256_add_epi32(sqrR, r2);

        ptrI++;
        ptrA++;
    }while (--lc);

    if (width % 8){
        PartialWordAccess32_x64_AVX2 loader(width % 8);

        __m256i p = loader.load_i32(ptrI);
        __m256i m = loader.load_i32(ptrA);

        m = _mm256_srai_epi32(m, 31);
        p = _mm256_and_si256(p, m);

        __m256i r0 = _mm256_and_si256(p, _mm256_set1_epi32(0x000000ff));
//        __m256i r1 = _mm256_and_si256(_mm256_srli_epi32(p, 8), _mm256_set1_epi32(0x000000ff));
//        __m256i r2 = _mm256_and_si256(_mm256_srli_epi32(p, 16), _mm256_set1_epi32(0x000000ff));
        __m256i r1 = _mm256_shuffle_epi8(p, _mm256_setr_epi8(
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
            1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
        ));
        __m256i r2 = _mm256_shuffle_epi8(p, _mm256_setr_epi8(
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1,
            2, -1, -1, -1, 6, -1, -1, -1, 10, -1, -1, -1, 14, -1, -1, -1
        ));

        sumB = _mm256_add_epi32(sumB, r0);
        sumG = _mm256_add_epi32(sumG, r1);
        sumR = _mm256_add_epi32(sumR, r2);
        sumA = _mm256_sub_epi32(sumA, m);

        r0 = _mm256_mullo_epi16(r0, r0);
        r1 = _mm256_mullo_epi16(r1, r1);
        r2 = _mm256_mullo_epi16(r2, r2);

        sqrB = _mm256_add_epi32(sqrB, r0);
        sqrG = _mm256_add_epi32(sqrG, r1);
        sqrR = _mm256_add_epi32(sqrR, r2);
    }

    sums.count += reduce_add32_x64_AVX2(sumA);
    sums.sumR += reduce_add32_x64_AVX2(sumR);
    sums.sumG += reduce_add32_x64_AVX2(sumG);
    sums.sumB += reduce_add32_x64_AVX2(sumB);
    sums.sqrR += reduce_add32_x64_AVX2(sqrR);
    sums.sqrG += reduce_add32_x64_AVX2(sqrG);
    sums.sqrB += reduce_add32_x64_AVX2(sqrB);
}
void pixel_sum_sqr_x64_AVX2(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
    if (width < 8){
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
        pixel_sum_sqr_x64_AVX2(sums, (uint16_t)width, image, alpha);
        image = (const uint32_t*)((const char*)image + image_bytes_per_row);
        alpha = (const uint32_t*)((const char*)alpha + alpha_bytes_per_row);
    }
}



}
}
#endif
