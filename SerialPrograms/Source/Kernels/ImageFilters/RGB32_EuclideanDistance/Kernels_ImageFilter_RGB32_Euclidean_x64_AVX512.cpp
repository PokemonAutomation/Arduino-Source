/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX512.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

namespace PokemonAutomation{
namespace Kernels{




class PixelTest_Rgb32Euclidean_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Euclidean_x64_AVX512(
        uint32_t expected, double max_euclidean_distance
    )
        : m_expected_ag(_mm512_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm512_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm512_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __mmask16 test_word(__m512i pixel) const{
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

        return _mm512_cmpgt_epi32_mask(m_distance_squared, sum_sqr);
    }

private:
    const __m512i m_expected_ag;
    const __m512i m_expected_rb;
    const __m512i m_distance_squared;
};



size_t filter_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_x64_AVX512 tester(
        expected, max_euclidean_distance
    );
    FilterImage_Rgb32_x64_AVX512<PixelTest_Rgb32Euclidean_x64_AVX512> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_x64_AVX512 tester(
        expected, max_euclidean_distance
    );
    ToBlackWhite_Rgb32_x64_AVX512<PixelTest_Rgb32Euclidean_x64_AVX512> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
#endif
