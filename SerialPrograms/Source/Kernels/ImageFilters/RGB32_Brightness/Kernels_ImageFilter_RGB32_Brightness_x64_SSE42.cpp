/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <limits>
#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_SSE42.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class PixelTest_Rgb32Brightness_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Brightness_x64_SSE42(
        Rgb32BrightnessWeights weights, int32_t min_sum, int32_t max_sum
    )
        : m_weightsL(_mm_srai_epi16(_mm_set1_epi32((weights.u32 << 8) & 0xff00ff00), 8))
        , m_weightsH(_mm_srai_epi16(_mm_set1_epi32((weights.u32     ) & 0xff00ff00), 8))
        , m_min(_mm_set1_epi32(min_sum == std::numeric_limits<int32_t>::min() ? min_sum : min_sum - 1))
        , m_max(_mm_set1_epi32(max_sum == std::numeric_limits<int32_t>::max() ? max_sum : max_sum + 1))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m128i test_word(__m128i pixel) const{
        __m128i rb = _mm_and_si128(pixel, _mm_set1_epi32(0x00ff00ff));
        __m128i ag = _mm_and_si128(_mm_srli_epi16(pixel, 8), _mm_set1_epi32(0x00ff00ff));

        rb = _mm_madd_epi16(rb, m_weightsL);
        ag = _mm_madd_epi16(ag, m_weightsH);
        pixel = _mm_add_epi32(rb, ag);

        __m128i cmp0 = _mm_cmpgt_epi32(pixel, m_min);
        __m128i cmp1 = _mm_cmpgt_epi32(m_max, pixel);
        return _mm_and_si128(cmp0, cmp1);
    }

private:
    const __m128i m_weightsL;
    const __m128i m_weightsH;
    const __m128i m_min;
    const __m128i m_max;
};




size_t filter_rgb32_brightness_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_SSE42 tester(
        weights,
        min_brightness, max_brightness
    );
    FilterImage_Rgb32_x64_SSE42<PixelTest_Rgb32Brightness_x64_SSE42> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_brightness_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_SSE42 tester(
        weights,
        min_brightness, max_brightness
    );
    ToBlackWhite_Rgb32_x64_SSE42<PixelTest_Rgb32Brightness_x64_SSE42> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}








}
}
#endif
