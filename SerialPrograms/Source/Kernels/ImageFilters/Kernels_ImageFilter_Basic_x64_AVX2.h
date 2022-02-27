/*  Image Filters Basic (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_x64_AVX2_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_x64_AVX2_H

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;

public:
    ImageFilter_RgbRange_x64_AVX2(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(invert ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
        __m256i pixel = _mm256_loadu_si256((const __m256i*)in);
        pixel = process_word(pixel);
        _mm256_storeu_si256((__m256i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        PartialWordLoader32_x64_AVX2 loader(left);
        __m256i pixel = loader.load(in);
        pixel = process_word(pixel);
        loader.store(out, pixel);
    }

private:
    PA_FORCE_INLINE __m256i process_word(__m256i pixel) const{
        __m256i adj = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, adj);
        __m256i cmp1 = _mm256_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        cmp0 = _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
        cmp0 = _mm256_xor_si256(cmp0, m_invert);
        return _mm256_blendv_epi8(m_replacement, pixel, cmp0);
    }

private:
    __m256i m_replacement;
    __m256i m_invert;
    __m256i m_mins;
    __m256i m_maxs;
};



}
}
#endif
