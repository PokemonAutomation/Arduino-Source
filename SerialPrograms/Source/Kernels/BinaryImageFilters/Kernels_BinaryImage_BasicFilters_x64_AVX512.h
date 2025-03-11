/*  Binary Image Basic Filters (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX512_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_x64_AVX512_H

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class FilterByMask_x64_AVX512{
public:
    FilterByMask_x64_AVX512(uint32_t replacement, bool replace_if_zero)
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_replace_if_zero(replace_if_zero ? 0xffff : 0)
    {}

    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count = 64) const{
        size_t lc = count / 16;
        while (lc--){
            __m512i pixel = _mm512_loadu_si512((const __m512i*)pixels);
            pixel = filter16(bits, pixel);
            _mm512_storeu_si512((__m512i*)pixels, pixel);
            pixels += 16;
            bits >>= 16;
        }
        count %= 16;
        if (count){
            uint64_t mask = ((uint64_t)1 << count) - 1;
            __m512i pixel = _mm512_maskz_loadu_epi32((__mmask16)mask, pixels);
            pixel = filter16(bits, pixel);
            _mm512_mask_storeu_epi32(pixels, (__mmask16)mask, pixel);
        }
    }

private:
    PA_FORCE_INLINE __m512i filter16(uint64_t mask, __m512i pixel) const{
        mask ^= m_replace_if_zero;
        return _mm512_mask_blend_epi32((__mmask16)mask, pixel, m_replacement);
    }

private:
    __m512i m_replacement;
    uint32_t m_replace_if_zero;
};



class Compressor_RgbRange_x64_AVX512{
public:
    Compressor_RgbRange_x64_AVX512(uint32_t mins, uint32_t maxs)
        : m_mins(_mm512_set1_epi32(mins))
        , m_maxs(_mm512_set1_epi32(maxs))
    {}

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
        while (lc--){
            __m512i pixel = _mm512_loadu_si512((const __m512i*)pixels);
            bits |= convert16(pixel) << c;
            pixels += 16;
            c += 16;
        }
        count %= 16;
        if (count){
            uint64_t mask = ((uint64_t)1 << count) - 1;
            __m512i pixel = _mm512_maskz_loadu_epi32((__mmask16)mask, pixels);
            bits |= (convert16(pixel) & mask) << c;
        }
//        cout << "bits = " << bits << endl;
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert16(__m512i pixel) const{
#if 0
        __mmask64 cmp64A = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_cmpgt_epu8_mask(pixel, m_maxs);
        pixel = _mm512_movm_epi8(cmp64A | cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(pixel, _mm512_setzero_si512());
#else
        __mmask64 cmp64A = _mm512_cmple_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_mask_cmple_epu8_mask(cmp64A, pixel, m_maxs);
        pixel = _mm512_movm_epi8(cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(pixel, _mm512_set1_epi32(-1));
#endif
        return cmp16;
    }

private:
    __m512i m_mins;
    __m512i m_maxs;
};



class Compressor_RgbEuclidean_x64_AVX512{
public:
    Compressor_RgbEuclidean_x64_AVX512(uint32_t expected, double max_euclidean_distance)
        : m_expected_ag(_mm512_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm512_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm512_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

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
        while (lc--){
            __m512i pixel = _mm512_loadu_si512((const __m512i*)pixels);
            bits |= convert16(pixel) << c;
            pixels += 16;
            c += 16;
        }
        count %= 16;
        if (count){
            uint64_t mask = ((uint64_t)1 << count) - 1;
            __m512i pixel = _mm512_maskz_loadu_epi32((__mmask16)mask, pixels);
            bits |= (convert16(pixel) & mask) << c;
        }
//        cout << "bits = " << bits << endl;
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert16(__m512i pixel) const{
        __m512i ag = _mm512_and_si512(_mm512_srli_epi16(pixel, 8), _mm512_set1_epi32(0x000000ff));
        __m512i rb = _mm512_and_si512(pixel, _mm512_set1_epi32(0x00ff00ff));

        ag = _mm512_sub_epi16(ag, m_expected_ag);
        rb = _mm512_sub_epi16(rb, m_expected_rb);

        __m512i g = _mm512_mullo_epi16(ag, ag);
        rb = _mm512_mullo_epi16(rb, rb);
        __m512i r = _mm512_srli_epi32(rb, 16);
        __m512i b = _mm512_and_si512(rb, _mm512_set1_epi32(0x0000ffff));

        __m512i sum_sqr = _mm512_add_epi32(r, g);
        sum_sqr = _mm512_add_epi32(sum_sqr, b);

        __mmask16 cmp = _mm512_cmpgt_epi32_mask(m_distance_squared, sum_sqr);
        return cmp;
    }

private:
    const __m512i m_expected_ag;
    const __m512i m_expected_rb;
    const __m512i m_distance_squared;
};




}
}
#endif
