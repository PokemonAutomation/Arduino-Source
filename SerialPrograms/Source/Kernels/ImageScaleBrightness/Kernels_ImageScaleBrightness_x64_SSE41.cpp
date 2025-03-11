/*  Scale Brightness (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <stdint.h>
#include <smmintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE void scale_brightness_x64_SSE41(
    size_t width, uint32_t* image,
    __m128 scale
){
    for (size_t c = 0; c < width; c++){
        uint32_t pixel = image[c];

        __m128i pi = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(pixel));
        __m128 pf = _mm_cvtepi32_ps(pi);
        pf = _mm_mul_ps(pf, scale);
        pf = _mm_min_ps(pf, _mm_set1_ps(255.));
        pf = _mm_max_ps(pf, _mm_set1_ps(0.));
        pi = _mm_cvtps_epi32(pf);
        pi = _mm_shuffle_epi8(pi, _mm_setr_epi8(0, 4, 8, 12, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1));

        image[c] = _mm_cvtsi128_si32(pi);
    }
}
void scale_brightness_x64_SSE41(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
    if (width == 0 || height == 0){
        return;
    }
    __m128 scale = _mm_setr_ps(scaleB, scaleG, scaleR, 1);
    for (uint16_t r = 0; r < height; r++){
        scale_brightness_x64_SSE41(width, image, scale);
        image = (uint32_t*)((char*)image + bytes_per_row);
    }
}



}
}
#endif
