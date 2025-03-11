/*  Sum of Squares of Deviation (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_ImagePixelSumSqrDev.h"

namespace PokemonAutomation{
namespace Kernels{


template <SumSquareMode mode>
void sum_sqr_deviation_Default(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);



template <SumSquareMode mode>
PA_FORCE_INLINE void sum_sqr_deviation_x64_AVX2(
    __m256i& total, __m256i& sum,
    __m256i r, __m256i i,
    __m256i background = _mm256_setzero_si256()
){
    __m256i alphaR = _mm256_srai_epi32(r, 31);

    if (mode == SumSquareMode::REFERENCE_ALPHA){
        r = _mm256_and_si256(r, alphaR);
        i = _mm256_and_si256(i, alphaR);
    }
    if (mode == SumSquareMode::USE_BACKGROUND){
        r = _mm256_blendv_epi8(background, r, alphaR);
    }
    if (mode == SumSquareMode::ARBITRATE_ALPHAS){
        __m256i alphaI = _mm256_srai_epi32(i, 31);
        r = _mm256_and_si256(r, alphaR);
        i = _mm256_and_si256(i, alphaI);
        alphaI = _mm256_xor_si256(alphaI, alphaR);
        r = _mm256_or_si256(r, alphaI);
        i = _mm256_andnot_si256(alphaI, i);
    }

    __m256i r0 = _mm256_and_si256(r, _mm256_set1_epi32(0x00ff00ff));
    __m256i i0 = _mm256_and_si256(i, _mm256_set1_epi32(0x00ff00ff));
//    __m256i r1 = _mm256_and_si256(_mm256_srli_epi32(r, 8), _mm256_set1_epi32(0x000000ff));
//    __m256i i1 = _mm256_and_si256(_mm256_srli_epi32(i, 8), _mm256_set1_epi32(0x000000ff));
    __m256i r1 = _mm256_shuffle_epi8(r, _mm256_setr_epi8(
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
    ));
    __m256i i1 = _mm256_shuffle_epi8(i, _mm256_setr_epi8(
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
    ));

    r0 = _mm256_sub_epi16(r0, i0);
    r1 = _mm256_sub_epi16(r1, i1);

    r0 = _mm256_madd_epi16(r0, r0);
    r1 = _mm256_madd_epi16(r1, r1);

    total = _mm256_sub_epi32(total, alphaR);
    sum = _mm256_add_epi32(sum, r0);
    sum = _mm256_add_epi32(sum, r1);
}

template <SumSquareMode mode>
PA_FORCE_INLINE void sum_sqr_deviation_x64_AVX2(
    uint64_t& count, uint64_t& sumsqrs,
    uint16_t width,
    const uint32_t* ref, const uint32_t* img,
    __m256i background
){
    __m256i total = _mm256_setzero_si256();
    __m256i sum = _mm256_setzero_si256();

    const __m256i* ptrR = (const __m256i*)ref;
    const __m256i* ptrI = (const __m256i*)img;

    size_t lc = width / 8;
    do{
        __m256i r = _mm256_loadu_si256(ptrR);
        __m256i i = _mm256_loadu_si256(ptrI);
        sum_sqr_deviation_x64_AVX2<mode>(total, sum, r, i, background);
        ptrR++;
        ptrI++;
    }while (--lc);

    if (width % 8){
        __m256i mask = _mm256_cmpgt_epi32(
            _mm256_set1_epi32(width % 8),
            _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7)
        );
        __m256i r = _mm256_maskload_epi32((const int*)ptrR, mask);
        __m256i i = _mm256_maskload_epi32((const int*)ptrI, mask);

        background = _mm256_and_si256(background, mask);
        sum_sqr_deviation_x64_AVX2<mode>(total, sum, r, i, background);
    }

    count += reduce_add32_x64_AVX2(total);
    sumsqrs += reduce_add32_x64_AVX2(sum);
}


template <SumSquareMode mode>
void sum_sqr_deviation_x64_AVX2(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
){
    if (width < 8){
        sum_sqr_deviation_Default<mode>(
            count, sumsqrs,
            width, height,
            ref, ref_bytes_per_line,
            img, img_bytes_per_line,
            background
        );
        return;
    }
    if (width > 22017){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width limit exceeded: " + std::to_string(width));
    }
    __m256i vbackground = _mm256_set1_epi32(background);
    for (size_t r = 0; r < height; r++){
        sum_sqr_deviation_x64_AVX2<mode>(
            count, sumsqrs,
            (uint16_t)width, ref, img, vbackground
        );
        ref = (const uint32_t*)((const char*)ref + ref_bytes_per_line);
        img = (const uint32_t*)((const char*)img + img_bytes_per_line);
    }
}


template
void sum_sqr_deviation_x64_AVX2<SumSquareMode::REFERENCE_ALPHA>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_AVX2<SumSquareMode::USE_BACKGROUND>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_AVX2<SumSquareMode::ARBITRATE_ALPHAS>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);




}
}
#endif
