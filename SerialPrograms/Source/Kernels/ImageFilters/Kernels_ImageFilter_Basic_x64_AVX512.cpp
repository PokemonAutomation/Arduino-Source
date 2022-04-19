/*  Image Filters Basic (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_17_Skylake

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;

public:
    ImageFilter_RgbRange_x64_AVX512(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
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



void filter_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, out0, bytes_per_row0, filter0);
}
void filter2_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX512 filter1(mins1, maxs1, replacement1, invert1);
    filter2_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1
    );
}
void filter4_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX512 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_AVX512 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_AVX512 filter3(mins3, maxs3, replacement3, invert3);
    filter4_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1,
        out2, bytes_per_row2, filter3,
        out3, bytes_per_row3, filter1
    );
}



}
}
#endif
