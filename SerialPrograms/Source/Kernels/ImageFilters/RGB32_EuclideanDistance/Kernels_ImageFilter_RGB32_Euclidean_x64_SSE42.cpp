/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_SSE42.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{




class PixelTest_Rgb32Euclidean_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Euclidean_x64_SSE42(
        uint32_t expected, double max_euclidean_distance
    )
        : m_expected_g(_mm_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m128i test_word(__m128i pixel) const{
        // _mm_srli_epi16: Shift 16-bit integers in pixels right by 8 while shifting in zeros,
        // ng: green channels of each pixel, but shifted right by 8 bits
        __m128i ng = _mm_and_si128(_mm_srli_epi16(pixel, 8), _mm_set1_epi32(0x000000ff));
        // rb: the red and blue channels of each pixel
        __m128i rb = _mm_and_si128(pixel, _mm_set1_epi32(0x00ff00ff));
        // g: the difference between input pixel channels and the expected values
        ng = _mm_sub_epi16(ng, m_expected_g);
        rb = _mm_sub_epi16(rb, m_expected_rb);
        // compute square operation:
        // now each 16-bit region is a squared channel difference
        // here we assume alpha channel from input image is the same as the expected,
        // so the alpha channel difference is always 0, therefore:
        // g: each 32-bit integer contains the green channel squared difference
        __m128i g = _mm_mullo_epi16(ng, ng);
        rb = _mm_mullo_epi16(rb, rb);
        // r: each 32-bit integer contains the red channel squared difference
        __m128i r = _mm_srli_epi32(rb, 16);
        // b: each 32-bit integer contains the blue channel squared difference
        __m128i b = _mm_and_si128(rb, _mm_set1_epi32(0x0000ffff));
        // compute r^2 + g^2 + b^2
        __m128i sum_sqr = _mm_add_epi32(r, g);
        sum_sqr = _mm_add_epi32(sum_sqr, b);

        return _mm_cmpgt_epi32(m_distance_squared, sum_sqr);
    }

private:
    const __m128i m_expected_g;
    const __m128i m_expected_rb;
    const __m128i m_distance_squared;
};



size_t filter_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_x64_SSE42 tester(
        expected, max_euclidean_distance
    );
    FilterImage_Rgb32_x64_SSE42<PixelTest_Rgb32Euclidean_x64_SSE42> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_x64_SSE42 tester(
        expected, max_euclidean_distance
    );
    ToBlackWhite_Rgb32_x64_SSE42<PixelTest_Rgb32Euclidean_x64_SSE42> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}








}
}
#endif
