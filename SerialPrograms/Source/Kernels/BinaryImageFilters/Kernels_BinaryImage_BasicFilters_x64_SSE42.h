/*  Binary Image Basic Filters (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_SSE41_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_SSE41_H

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"

namespace PokemonAutomation{
namespace Kernels{



class Compressor_RgbRange_x64_SSE41{
public:
    Compressor_RgbRange_x64_SSE41(uint32_t mins, uint32_t maxs)
        : m_mins(_mm_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm_set1_epi32(maxs ^ 0x80808080))
    {}

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
        while (lc--){
            __m128i pixel = _mm_loadu_si128((const __m128i*)pixels);
            bits |= convert4(pixel) << c;
            pixels += 4;
            c += 4;
        }
        count %= 4;
        if (count){
            PartialWordAccess_x64_SSE41 loader(count * sizeof(uint32_t));
            __m128i pixel = loader.load(pixels);
            uint64_t mask = ((uint64_t)1 << count) - 1;
            bits |= (convert4(pixel) & mask) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert4(__m128i pixel) const{
        pixel = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp0 = _mm_cmpgt_epi8(m_mins, pixel);
        __m128i cmp1 = _mm_cmpgt_epi8(pixel, m_maxs);
        cmp0 = _mm_or_si128(cmp0, cmp1);
        cmp0 = _mm_cmpeq_epi32(cmp0, _mm_setzero_si128());
        return _mm_movemask_ps(_mm_castsi128_ps(cmp0));
    }

private:
    __m128i m_mins;
    __m128i m_maxs;
};



class Filter_RgbRange_x64_SSE41{
public:
    Filter_RgbRange_x64_SSE41(uint32_t replacement, bool replace_if_zero)
        : m_replacement(_mm_set1_epi32(replacement))
        , m_replace_if_zero(_mm_set1_epi32(replace_if_zero ? 0 : 0xffffffff))
    {}

    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count = 64) const{
        size_t lc = count / 4;
        while (lc--){
            __m128i pixel = _mm_loadu_si128((const __m128i*)pixels);
            pixel = filter4((uint32_t)bits & 15, pixel);
            _mm_storeu_si128((__m128i*)pixels, pixel);
            pixels += 4;
            bits >>= 4;
        }
        count %= 4;
        if (count){
            PartialWordAccess_x64_SSE41 loader(count * sizeof(uint32_t));
            __m128i pixel = loader.load(pixels);
            pixel = filter4((uint32_t)bits & 15, pixel);
            do{
                pixels[0] = _mm_cvtsi128_si32(pixel);
                pixel = _mm_srli_si128(pixel, 4);
                pixels++;
            }while(--count);
        }
    }

private:
    PA_FORCE_INLINE __m128i filter4(uint32_t bits, __m128i pixel) const{
        bits *= 0x01010101;
        bits &= 0x08040201;
        __m128i mask = _mm_cvtsi32_si128(bits);
        mask = _mm_cvtepu8_epi32(mask);
        mask = _mm_cmpeq_epi32(mask, _mm_setzero_si128());
        mask = _mm_xor_si128(mask, m_replace_if_zero);
        return _mm_blendv_epi8(pixel, m_replacement, mask);
    }

private:
    __m128i m_replacement;
    __m128i m_replace_if_zero;
};




}
}
#endif
