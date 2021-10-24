/*  Binary Image Basic Filters (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_SSE41_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_SSE41_H

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_BinaryImage_BasicFilters.h"

namespace PokemonAutomation{
namespace Kernels{


class MinRgbFilter_x64_SSE41{
public:
    MinRgbFilter_x64_SSE41(
        uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
    ){
        uint32_t smin =
            ((uint32_t)min_alpha << 24) |
            ((uint32_t)min_red << 16) |
            ((uint32_t)min_green << 8) |
            (uint32_t)min_blue;
        m_mins = _mm_set1_epi32(smin ^ 0x80808080);
    }

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        size_t c = 0;
        do{
            __m128i pixel = _mm_loadu_si128((const __m128i*)(pixels + c));
            bits |= convert4(pixel) << c;
            c += 4;
        }while (c < 64);
        return bits;
    }
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;
        size_t lc = count / 4;
        do{
            __m128i pixel = _mm_loadu_si128((const __m128i*)pixels);
            bits |= convert4(pixel) << c;
            pixels += 4;
            c += 4;
        }while (--lc);
        count %= 4;
        if (count){
            PartialWordLoader_x64_SSE41 loader(count * sizeof(uint32_t));
            __m128i pixel = loader.load_no_read_past_end(pixels);
            bits |= convert4(pixel) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert4(__m128i pixel) const{
        pixel = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp = _mm_cmpgt_epi8(m_mins, pixel);
        cmp = _mm_cmpeq_epi32(cmp, _mm_setzero_si128());
        return _mm_movemask_ps(_mm_castsi128_ps(cmp));
    }

private:
    __m128i m_mins;
};





}
}
#endif
