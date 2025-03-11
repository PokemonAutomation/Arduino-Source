/*  Sum of Squares of Deviation (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_ImagePixelSumSqrDev.h"

#include <iostream>
using std::cout;
using std::endl;

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
PA_FORCE_INLINE void sum_sqr_deviation_x64_AVX512(
    __m512i& total, __m512i& sum,
    __m512i r, __m512i i,
    __m512i background = _mm512_setzero_si512()
){
    __mmask16 alphaR = _mm512_movepi32_mask(r);
    __mmask16 alphaI;

    if (mode == SumSquareMode::USE_BACKGROUND){
        r = _mm512_mask_blend_epi32(alphaR, background, r);
    }
    if (mode == SumSquareMode::ARBITRATE_ALPHAS){
        alphaI = _mm512_movepi32_mask(i);
    }

    __m512i r0 = _mm512_and_si512(r, _mm512_set1_epi32(0x00ff00ff));
    __m512i i0 = _mm512_and_si512(i, _mm512_set1_epi32(0x00ff00ff));
//    __m512i r1 = _mm512_and_si512(_mm512_srli_epi32(r, 8), _mm512_set1_epi32(0x000000ff));
//    __m512i i1 = _mm512_and_si512(_mm512_srli_epi32(i, 8), _mm512_set1_epi32(0x000000ff));
    __m512i r1 = _mm512_shuffle_epi8(r, _mm512_setr_epi8(
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
    ));
    __m512i i1 = _mm512_shuffle_epi8(i, _mm512_setr_epi8(
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1,
        1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1
    ));

    r0 = _mm512_sub_epi16(r0, i0);
    r1 = _mm512_sub_epi16(r1, i1);

    r0 = _mm512_madd_epi16(r0, r0);
    r1 = _mm512_madd_epi16(r1, r1);

    r0 = _mm512_add_epi32(r0, r1);

    total = _mm512_mask_sub_epi32(total, alphaR, total, _mm512_set1_epi32(-1));

    if (mode == SumSquareMode::REFERENCE_ALPHA){
        sum = _mm512_mask_add_epi32(sum, alphaR, sum, r0);
    }
    if (mode == SumSquareMode::USE_BACKGROUND){
        sum = _mm512_add_epi32(sum, r0);
    }
    if (mode == SumSquareMode::ARBITRATE_ALPHAS){
        r0 = _mm512_mask_mov_epi32(r0, alphaR ^ alphaI, _mm512_set1_epi32(195075));
        sum = _mm512_mask_add_epi32(sum, alphaR | alphaI, sum, r0);
    }
}

template <SumSquareMode mode>
PA_FORCE_INLINE void sum_sqr_deviation_x64_AVX512(
    uint64_t& count, uint64_t& sumsqrs,
    uint16_t width,
    const uint32_t* ref, const uint32_t* img,
    __m512i background
){
    __m512i total = _mm512_setzero_si512();
    __m512i sum = _mm512_setzero_si512();

    const __m512i* ptrR = (const __m512i*)ref;
    const __m512i* ptrI = (const __m512i*)img;

    size_t lc = width / 16;
    do{
        __m512i r = _mm512_loadu_si512(ptrR);
        __m512i i = _mm512_loadu_si512(ptrI);
        sum_sqr_deviation_x64_AVX512<mode>(total, sum, r, i, background);
        ptrR++;
        ptrI++;
    }while (--lc);

    if (width % 16){
        __mmask16 mask = (((uint32_t)1 << (width % 16))) - 1;
        __m512i r = _mm512_maskz_loadu_epi32(mask, ptrR);
        __m512i i = _mm512_maskz_loadu_epi32(mask, ptrI);
        background = _mm512_maskz_mov_epi32(mask, background);
        sum_sqr_deviation_x64_AVX512<mode>(total, sum, r, i, background);
    }

    count += _mm512_reduce_add_epi32(total);
    sumsqrs += _mm512_reduce_add_epi32(sum);
}


template <SumSquareMode mode>
void sum_sqr_deviation_x64_AVX512(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
){
    if (width < 16){
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
    __m512i vbackground = _mm512_set1_epi32(background);
    for (size_t r = 0; r < height; r++){
        sum_sqr_deviation_x64_AVX512<mode>(
            count, sumsqrs,
            (uint16_t)width, ref, img, vbackground
        );
        ref = (const uint32_t*)((const char*)ref + ref_bytes_per_line);
        img = (const uint32_t*)((const char*)img + img_bytes_per_line);
    }
}


template
void sum_sqr_deviation_x64_AVX512<SumSquareMode::REFERENCE_ALPHA>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_AVX512<SumSquareMode::USE_BACKGROUND>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_AVX512<SumSquareMode::ARBITRATE_ALPHAS>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);




}
}
#endif
