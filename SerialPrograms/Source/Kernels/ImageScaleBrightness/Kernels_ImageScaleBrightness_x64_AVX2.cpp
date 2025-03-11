/*  Scale Brightness (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE void scale_brightness_x64_AVX2(
    size_t width, uint32_t* image,
    __m256 scale
){
    size_t lc = width / 2;
    do{
        __m128i pixel = _mm_loadl_epi64((const __m128i*)image);

        __m256i pi = _mm256_cvtepu8_epi32(pixel);
        __m256 pf = _mm256_cvtepi32_ps(pi);
        pf = _mm256_mul_ps(pf, scale);
        pf = _mm256_min_ps(pf, _mm256_set1_ps(255.));
        pf = _mm256_max_ps(pf, _mm256_set1_ps(0.));
        pi = _mm256_cvtps_epi32(pf);
        pi = _mm256_shuffle_epi8(pi, _mm256_setr_epi8(
            0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1
        ));

        pixel = _mm_or_si128(
            _mm256_castsi256_si128(pi),
            _mm256_extracti128_si256(pi, 1)
        );

        _mm_storel_epi64((__m128i*)image, pixel);
        image += 2;
    }while (--lc);

    if (width % 2){
        uint32_t pixel = image[0];

        __m128i pi = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(pixel));
        __m128 pf = _mm_cvtepi32_ps(pi);
        pf = _mm_mul_ps(pf, _mm256_castps256_ps128(scale));
        pf = _mm_min_ps(pf, _mm_set1_ps(255.));
        pf = _mm_max_ps(pf, _mm_set1_ps(0.));
        pi = _mm_cvtps_epi32(pf);
        pi = _mm_shuffle_epi8(pi, _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

        image[0] = _mm_cvtsi128_si32(pi);
    }
}
void scale_brightness_x64_AVX2(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
    if (width == 0 || height == 0){
        return;
    }
    __m256 scale = _mm256_setr_ps(scaleB, scaleG, scaleR, 1, scaleB, scaleG, scaleR, 1);
    for (uint16_t r = 0; r < height; r++){
        scale_brightness_x64_AVX2(width, image, scale);
        image = (uint32_t*)((char*)image + bytes_per_row);
    }
}



}
}
#endif
