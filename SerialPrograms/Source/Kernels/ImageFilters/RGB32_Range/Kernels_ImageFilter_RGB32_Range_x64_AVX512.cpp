/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Range.h"

namespace PokemonAutomation{
namespace Kernels{


namespace{

struct PartialWordMask{
    __mmask16 m;

    PA_FORCE_INLINE PartialWordMask(size_t left)
        : m(((__mmask16)1 << left) - 1)
    {}
};

}




class ImageFilterRunner_Rgb32Range_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ImageFilterRunner_Rgb32Range_x64_AVX512(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(invert ? 0xffff : 0)
        , m_mins(_mm512_set1_epi32(mins))
        , m_maxs(_mm512_set1_epi32(maxs))
        , m_count(_mm512_setzero_si512())
    {}
    ImageFilterRunner_Rgb32Range_x64_AVX512(FilterRgb32RangeFilter& filter)
        : ImageFilterRunner_Rgb32Range_x64_AVX512(filter.mins, filter.maxs, filter.replacement, filter.invert)
    {}

    PA_FORCE_INLINE size_t count() const{
        return _mm512_reduce_add_epi32(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        pixel = process_word(pixel);
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m512i pixel = _mm512_maskz_loadu_epi32(mask.m, in);
        pixel = process_word(pixel);
        _mm512_mask_storeu_epi32(out, mask.m, pixel);
    }

private:
    PA_FORCE_INLINE __m512i process_word(__m512i pixel){
#if 0
        __mmask64 cmp64A = _mm512_cmpgt_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_cmpgt_epu8_mask(pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64A | cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_setzero_si512());
#else
        __mmask64 cmp64A = _mm512_cmple_epu8_mask(m_mins, pixel);
        __mmask64 cmp64B = _mm512_mask_cmple_epu8_mask(cmp64A, pixel, m_maxs);
        __m512i mask = _mm512_movm_epi8(cmp64B);
        __mmask16 cmp16 = _mm512_cmpeq_epi32_mask(mask, _mm512_set1_epi32(-1));
#endif
        m_count = _mm512_mask_sub_epi32(m_count, cmp16, m_count, _mm512_set1_epi32(-1));
        cmp16 ^= m_invert;
        return _mm512_mask_blend_epi32(cmp16, m_replacement, pixel);
    }

private:
    const __m512i m_replacement;
    const __mmask16 m_invert;
    const __m512i m_mins;
    const __m512i m_maxs;
    __m512i m_count;
};





size_t filter_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row, uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert
){
    ImageFilterRunner_Rgb32Range_x64_AVX512 filter(mins, maxs, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilterRunner_Rgb32Range_x64_AVX512>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}




}
}
#endif
