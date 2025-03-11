/*  Scale Brightness (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE void scale_brightness_x64_AVX512(
    size_t width, uint32_t* image,
    __m512 scale
){
    size_t lc = width / 4;
    do{
        __m128i pixel = _mm_loadu_si128((const __m128i*)image);

        __m512i pi = _mm512_cvtepu8_epi32(pixel);
        __m512 pf = _mm512_cvtepi32_ps(pi);
        pf = _mm512_mul_ps(pf, scale);
        pf = _mm512_min_ps(pf, _mm512_set1_ps(255.));
        pf = _mm512_max_ps(pf, _mm512_set1_ps(0.));
        pi = _mm512_cvtps_epi32(pf);
        pixel = _mm512_cvtepi32_epi8(pi);

        _mm_storeu_si128((__m128i*)image, pixel);
        image += 4;
    }while (--lc);

    if (width % 4){
        __mmask8 mask = ((uint32_t)1 << (width % 4)) - 1;

        __m128i pixel = _mm_maskz_loadu_epi32(mask, (const __m128i*)image);

        __m512i pi = _mm512_cvtepu8_epi32(pixel);
        __m512 pf = _mm512_cvtepi32_ps(pi);
        pf = _mm512_mul_ps(pf, scale);
        pf = _mm512_min_ps(pf, _mm512_set1_ps(255.));
        pf = _mm512_max_ps(pf, _mm512_set1_ps(0.));
        pi = _mm512_cvtps_epi32(pf);
        pixel = _mm512_cvtepi32_epi8(pi);

        _mm_mask_storeu_epi32((__m128i*)image, mask, pixel);
    }
}
void scale_brightness_x64_AVX512(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
    if (width == 0 || height == 0){
        return;
    }
    __m512 scale = _mm512_setr_ps(
        scaleB, scaleG, scaleR, 1,
        scaleB, scaleG, scaleR, 1,
        scaleB, scaleG, scaleR, 1,
        scaleB, scaleG, scaleR, 1
    );
    for (uint16_t r = 0; r < height; r++){
        scale_brightness_x64_AVX512(width, image, scale);
        image = (uint32_t*)((char*)image + bytes_per_row);
    }
}



}
}
#endif
