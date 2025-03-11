/*  Sum of Squares of Deviation (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <smmintrin.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
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
PA_FORCE_INLINE void sum_sqr_deviation_x64_SSE41(
    __m128i& total, __m128i& sum,
    __m128i r, __m128i i,
    __m128i background = _mm_setzero_si128()
){
    __m128i alphaR = _mm_srai_epi32(r, 31);

    if (mode == SumSquareMode::REFERENCE_ALPHA){
        r = _mm_and_si128(r, alphaR);
        i = _mm_and_si128(i, alphaR);
    }
    if (mode == SumSquareMode::USE_BACKGROUND){
        r = _mm_blendv_epi8(background, r, alphaR);
    }
    if (mode == SumSquareMode::ARBITRATE_ALPHAS){
        __m128i alphaI = _mm_srai_epi32(i, 31);
        r = _mm_and_si128(r, alphaR);
        i = _mm_and_si128(i, alphaI);
        alphaI = _mm_xor_si128(alphaI, alphaR);
        r = _mm_or_si128(r, alphaI);
        i = _mm_andnot_si128(alphaI, i);
    }

    __m128i r0 = _mm_and_si128(r, _mm_set1_epi32(0x00ff00ff));
    __m128i i0 = _mm_and_si128(i, _mm_set1_epi32(0x00ff00ff));
//    __m128i r1 = _mm_and_si128(_mm_srli_epi32(r, 8), _mm_set1_epi32(0x000000ff));
//    __m128i i1 = _mm_and_si128(_mm_srli_epi32(i, 8), _mm_set1_epi32(0x000000ff));
    __m128i r1 = _mm_shuffle_epi8(r, _mm_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1));
    __m128i i1 = _mm_shuffle_epi8(i, _mm_setr_epi8(1, -1, -1, -1, 5, -1, -1, -1, 9, -1, -1, -1, 13, -1, -1, -1));

    r0 = _mm_sub_epi16(r0, i0);
    r1 = _mm_sub_epi16(r1, i1);

    r0 = _mm_madd_epi16(r0, r0);
    r1 = _mm_madd_epi16(r1, r1);

    total = _mm_sub_epi32(total, alphaR);
    sum = _mm_add_epi32(sum, r0);
    sum = _mm_add_epi32(sum, r1);
}

template <SumSquareMode mode>
PA_FORCE_INLINE void sum_sqr_deviation_x64_SSE41(
    uint64_t& count, uint64_t& sumsqrs,
    uint16_t width,
    const uint32_t* ref, const uint32_t* img,
    __m128i background
){
    __m128i total = _mm_setzero_si128();
    __m128i sum = _mm_setzero_si128();

    const __m128i* ptrR = (const __m128i*)ref;
    const __m128i* ptrI = (const __m128i*)img;

    size_t lc = width / 4;
    do{
        __m128i r = _mm_loadu_si128(ptrR);
        __m128i i = _mm_loadu_si128(ptrI);
        sum_sqr_deviation_x64_SSE41<mode>(total, sum, r, i, background);
        ptrR++;
        ptrI++;
    }while (--lc);

    if (width % 4){
#if 0
        PartialWordLoader_SSE4 loader(width * sizeof(uint32_t) % 16);

        __m128i p = loader.load_no_read_past_end(ptrI);
        __m128i m = loader.load_no_read_past_end(ptrA);

#else
        __m128i r = _mm_loadu_si128((const __m128i*)(ref + width - 4));
        __m128i i = _mm_loadu_si128((const __m128i*)(img + width - 4));

        uint8_t shift = (uint8_t)(ref + width - (const uint32_t*)ptrR);

        __m128i s = _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
        s = _mm_add_epi8(s, _mm_set1_epi8(128 - 4*shift));

        r = _mm_shuffle_epi8(r, s);
        i = _mm_shuffle_epi8(i, s);
        background = _mm_shuffle_epi8(background, s);
#endif

        sum_sqr_deviation_x64_SSE41<mode>(total, sum, r, i, background);
    }

    count += reduce32_x64_SSE41(total);
    sumsqrs += reduce32_x64_SSE41(sum);
}


template <SumSquareMode mode>
void sum_sqr_deviation_x64_SSE41(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
){
    if (width < 4){
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
    __m128i vbackground = _mm_set1_epi32(background);
    for (size_t r = 0; r < height; r++){
        sum_sqr_deviation_x64_SSE41<mode>(
            count, sumsqrs,
            (uint16_t)width, ref, img, vbackground
        );
        ref = (const uint32_t*)((const char*)ref + ref_bytes_per_line);
        img = (const uint32_t*)((const char*)img + img_bytes_per_line);
    }
}


template
void sum_sqr_deviation_x64_SSE41<SumSquareMode::REFERENCE_ALPHA>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_SSE41<SumSquareMode::USE_BACKGROUND>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);
template
void sum_sqr_deviation_x64_SSE41<SumSquareMode::ARBITRATE_ALPHAS>(
    uint64_t& count, uint64_t& sumsqrs,
    size_t width, size_t height,
    const uint32_t* ref, size_t ref_bytes_per_line,
    const uint32_t* img, size_t img_bytes_per_line,
    uint32_t background
);




}
}
#endif
