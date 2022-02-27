/*  Image Filters Basic (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_x64_SSE42_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_x64_SSE42_H

#include <stdint.h>
#include <cstddef>
#include <nmmintrin.h>
#include "Common/Compiler.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;

public:
    ImageFilter_RgbRange_x64_SSE42(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm_set1_epi32(replacement))
        , m_invert(invert ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_mins(_mm_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm_set1_epi32(maxs ^ 0x80808080))
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        pixel = process_word(pixel);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        PartialWordLoader_x64_SSE41 loader(left * sizeof(uint32_t));
        __m128i pixel = loader.load(in);
        pixel = process_word(pixel);
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    PA_FORCE_INLINE __m128i process_word(__m128i pixel) const{
        __m128i adj = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp0 = _mm_cmpgt_epi8(m_mins, adj);
        __m128i cmp1 = _mm_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm_or_si128(cmp0, cmp1);
        cmp0 = _mm_cmpeq_epi32(cmp0, _mm_setzero_si128());
        cmp0 = _mm_xor_si128(cmp0, m_invert);
        return _mm_blendv_epi8(m_replacement, pixel, cmp0);
    }

private:
    __m128i m_replacement;
    __m128i m_invert;
    __m128i m_mins;
    __m128i m_maxs;
};



}
}
#endif
