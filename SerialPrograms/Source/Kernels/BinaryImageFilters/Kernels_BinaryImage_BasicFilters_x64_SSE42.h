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



class FilterByMask_x64_SSE41{
public:
    FilterByMask_x64_SSE41(uint32_t replacement, bool replace_if_zero)
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



class Compressor_RgbEuclidean_x64_SSE41{
public:
    Compressor_RgbEuclidean_x64_SSE41(uint32_t expected, double max_euclidean_distance)
        : m_expected_ag(_mm_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
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
        __m128i ag = _mm_and_si128(_mm_srli_epi16(pixel, 8), _mm_set1_epi32(0x000000ff));
        __m128i rb = _mm_and_si128(pixel, _mm_set1_epi32(0x00ff00ff));

        ag = _mm_sub_epi16(ag, m_expected_ag);
        rb = _mm_sub_epi16(rb, m_expected_rb);

        __m128i g = _mm_mullo_epi16(ag, ag);
        rb = _mm_mullo_epi16(rb, rb);
        __m128i r = _mm_srli_epi32(rb, 16);
        __m128i b = _mm_and_si128(rb, _mm_set1_epi32(0x0000ffff));

        __m128i sum_sqr = _mm_add_epi32(r, g);
        sum_sqr = _mm_add_epi32(sum_sqr, b);

        __m128i cmp = _mm_cmpgt_epi32(m_distance_squared, sum_sqr);
        return _mm_movemask_ps(_mm_castsi128_ps(cmp));
    }

private:
    const __m128i m_expected_ag;
    const __m128i m_expected_rb;
    const __m128i m_distance_squared;
};




}
}
#endif
