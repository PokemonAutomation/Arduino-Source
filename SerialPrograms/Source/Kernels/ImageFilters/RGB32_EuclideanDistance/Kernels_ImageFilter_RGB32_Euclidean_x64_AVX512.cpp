/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

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



class ImageFilterRunner_Rgb32Euclidean_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ImageFilterRunner_Rgb32Euclidean_x64_AVX512(
        uint32_t replacement, bool replace_color_within_range,
        uint32_t expected, double max_euclidean_distance
    )
        : m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? 0xffff : 0)
        , m_expected_ag(_mm512_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm512_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm512_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
        , m_count(_mm512_setzero_si512())
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

        m_count = _mm512_mask_sub_epi32(m_count, cmp, m_count, _mm512_set1_epi32(-1));
        cmp ^= m_invert;
        return _mm512_mask_blend_epi32(cmp, m_replacement, pixel);
    }

private:
    const __m512i m_replacement;
    const __mmask16 m_invert;
    const __m512i m_expected_ag;
    const __m512i m_expected_rb;
    const __m512i m_distance_squared;
    __m512i m_count;
};
size_t filter_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance, void*
){
    ImageFilterRunner_Rgb32Euclidean_x64_AVX512 filter(
        replacement, replace_color_within_range,
        expected, max_euclidean_distance
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
#endif
