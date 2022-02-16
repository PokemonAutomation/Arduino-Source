/*  Image Filters Basic (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_x64_AVX512_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_x64_AVX512_H

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;

public:
    ImageFilter_RgbRange_x64_AVX512(uint32_t replacement, uint32_t mins, uint32_t maxs, bool invert)
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(invert ? 0xffff : 0)
        , m_mins(_mm512_set1_epi32(mins))
        , m_maxs(_mm512_set1_epi32(maxs))
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        pixel = process_word(pixel);
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        uint64_t mask = ((uint64_t)1 << left) - 1;
        __m512i pixel = _mm512_maskz_load_epi32((__mmask16)mask, in);
        pixel = process_word(pixel);
        _mm512_mask_storeu_epi32(out, (__mmask16)mask, pixel);
    }

private:
    PA_FORCE_INLINE __m512i process_word(__m512i pixel) const{
        __mmask64 cmp64A = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_cmpgt_epu8_mask(pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64A | cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_setzero_si512());
        cmp16 ^= m_invert;
        return _mm512_mask_blend_epi32(cmp16, m_replacement, pixel);
    }

private:
    __m512i m_replacement;
    __mmask16 m_invert;
    __m512i m_mins;
    __m512i m_maxs;
};



}
}
#endif
