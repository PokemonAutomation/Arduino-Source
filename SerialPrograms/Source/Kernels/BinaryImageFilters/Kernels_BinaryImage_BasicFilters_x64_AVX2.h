/*  Binary Image Basic Filters (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX2_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX2_H

#include <stdint.h>
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{



class FilterByMask_x64_AVX2{
public:
    FilterByMask_x64_AVX2(uint32_t replacement, bool replace_if_zero)
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_replace_if_zero(_mm256_set1_epi32(replace_if_zero ? 0 : 0xffffffff))
    {}

    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count = 64) const{
        size_t lc = count / 8;
        while (lc--){
            __m256i pixel = _mm256_loadu_si256((const __m256i*)pixels);
            pixel = filter8(bits & 255, pixel);
            _mm256_storeu_si256((__m256i*)pixels, pixel);
            pixels += 8;
            bits >>= 8;
        }
        count %= 8;
        if (count){
            PartialWordAccess32_x64_AVX2 loader(count);
            __m256i pixel = loader.load_i32(pixels);
            pixel = filter8(bits & 255, pixel);
            loader.store(pixels, pixel);
        }
    }

private:
    PA_FORCE_INLINE __m256i filter8(uint64_t bits, __m256i pixel) const{
        bits *= 0x0101010101010101;
        bits &= 0x8040201008040201;
        __m256i mask = _mm256_cvtepu8_epi32(_mm_cvtsi64_si128(bits));
        mask = _mm256_cmpeq_epi32(mask, _mm256_setzero_si256());
        mask = _mm256_xor_si256(mask, m_replace_if_zero);
        return _mm256_blendv_epi8(pixel, m_replacement, mask);
    }

private:
    __m256i m_replacement;
    __m256i m_replace_if_zero;
};



class Compressor_RgbRange_x64_AVX2{
public:
    Compressor_RgbRange_x64_AVX2(uint32_t mins, uint32_t maxs)
        : m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
    {}

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels +  0))) <<  0;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels +  8))) <<  8;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 16))) << 16;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 24))) << 24;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 32))) << 32;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 40))) << 40;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 48))) << 48;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 56))) << 56;
        return bits;
    }
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;
        size_t lc = count / 8;
        while (lc--){
            __m256i pixel = _mm256_loadu_si256((const __m256i*)pixels);
            bits |= convert8(pixel) << c;
            pixels += 8;
            c += 8;
        }
        count %= 8;
        if (count){
            PartialWordAccess32_x64_AVX2 loader(count);
            __m256i pixel = loader.load_i32(pixels);
            uint64_t mask = ((uint64_t)1 << count) - 1;
            bits |= (convert8(pixel) & mask) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert8(__m256i pixel) const{
        pixel = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, pixel);
        __m256i cmp1 = _mm256_cmpgt_epi8(pixel, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        cmp0 = _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
        return _mm256_movemask_ps(_mm256_castsi256_ps(cmp0));
    }

private:
    __m256i m_mins;
    __m256i m_maxs;
};



class Compressor_RgbEuclidean_x64_AVX2{
public:
    Compressor_RgbEuclidean_x64_AVX2(uint32_t expected, double max_euclidean_distance)
        : m_expected_ag(_mm256_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm256_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm256_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels +  0))) <<  0;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels +  8))) <<  8;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 16))) << 16;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 24))) << 24;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 32))) << 32;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 40))) << 40;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 48))) << 48;
        bits |= convert8(_mm256_loadu_si256((const __m256i*)(pixels + 56))) << 56;
        return bits;
    }
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;
        size_t lc = count / 8;
        while (lc--){
            __m256i pixel = _mm256_loadu_si256((const __m256i*)pixels);
            bits |= convert8(pixel) << c;
            pixels += 8;
            c += 8;
        }
        count %= 8;
        if (count){
            PartialWordAccess32_x64_AVX2 loader(count);
            __m256i pixel = loader.load_i32(pixels);
            uint64_t mask = ((uint64_t)1 << count) - 1;
            bits |= (convert8(pixel) & mask) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert8(__m256i pixel) const{
        __m256i ag = _mm256_and_si256(_mm256_srli_epi16(pixel, 8), _mm256_set1_epi32(0x000000ff));
        __m256i rb = _mm256_and_si256(pixel, _mm256_set1_epi32(0x00ff00ff));

        ag = _mm256_sub_epi16(ag, m_expected_ag);
        rb = _mm256_sub_epi16(rb, m_expected_rb);

        __m256i g = _mm256_mullo_epi16(ag, ag);
        rb = _mm256_mullo_epi16(rb, rb);
        __m256i r = _mm256_srli_epi32(rb, 16);
        __m256i b = _mm256_and_si256(rb, _mm256_set1_epi32(0x0000ffff));

        __m256i sum_sqr = _mm256_add_epi32(r, g);
        sum_sqr = _mm256_add_epi32(sum_sqr, b);

        __m256i cmp = _mm256_cmpgt_epi32(m_distance_squared, sum_sqr);
        return _mm256_movemask_ps(_mm256_castsi256_ps(cmp));
    }

private:
    const __m256i m_expected_ag;
    const __m256i m_expected_rb;
    const __m256i m_distance_squared;
};



}
}
#endif
