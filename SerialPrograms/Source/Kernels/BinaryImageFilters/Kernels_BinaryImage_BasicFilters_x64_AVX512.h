/*  Binary Image Basic Filters (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX512_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX512_H

#include <immintrin.h>
#include "Kernels_BinaryImage_BasicFilters.h"

namespace PokemonAutomation{
namespace Kernels{


class MinRgbFilter_x64_AVX512{
public:
    MinRgbFilter_x64_AVX512(
        uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
    ){
        uint32_t smin =
            ((uint32_t)min_alpha << 24) |
            ((uint32_t)min_red << 16) |
            ((uint32_t)min_green << 8) |
            (uint32_t)min_blue;
        m_mins = _mm512_set1_epi32(smin);
    }

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        bits |= convert16(_mm512_loadu_si512((const __m512i*)(pixels +  0))) <<  0;
        bits |= convert16(_mm512_loadu_si512((const __m512i*)(pixels + 16))) << 16;
        bits |= convert16(_mm512_loadu_si512((const __m512i*)(pixels + 32))) << 32;
        bits |= convert16(_mm512_loadu_si512((const __m512i*)(pixels + 48))) << 48;
        return bits;
    }
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;
        size_t lc = count / 16;
        do{
            __m512i pixel = _mm512_loadu_si512((const __m512i*)pixels);
            bits |= convert16(pixel) << c;
            pixels += 16;
            c += 16;
        }while (--lc);
        count %= 16;
        if (count){
            __mmask16 mask = ((uint32_t)1 << count) - 1;
            __m512i pixel = _mm512_maskz_load_epi32(mask, pixels);
            bits |= convert16(pixel) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert16(__m512i pixel) const{
        __mmask64 cmp64 = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        pixel = _mm512_movm_epi8(cmp64);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(pixel, _mm512_setzero_si512());
        return cmp16;
    }

private:
    __m512i m_mins;
};



}
}
#endif
