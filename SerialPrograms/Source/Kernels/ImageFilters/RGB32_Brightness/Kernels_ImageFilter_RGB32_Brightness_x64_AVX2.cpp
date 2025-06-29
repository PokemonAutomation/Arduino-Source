/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <limits>
#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX2.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class PixelTest_Rgb32Brightness_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Brightness_x64_AVX2(
        Rgb32BrightnessWeights weights, int32_t min_sum, int32_t max_sum
    )
        : m_weightsL(_mm256_srai_epi16(_mm256_set1_epi32((weights.u32 << 8) & 0xff00ff00), 8))
        , m_weightsH(_mm256_srai_epi16(_mm256_set1_epi32((weights.u32     ) & 0xff00ff00), 8))
        , m_min(_mm256_set1_epi32(min_sum == std::numeric_limits<int32_t>::min() ? min_sum : min_sum - 1))
        , m_max(_mm256_set1_epi32(max_sum == std::numeric_limits<int32_t>::max() ? max_sum : max_sum + 1))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m256i test_word(__m256i pixel) const{
        __m256i rb = _mm256_and_si256(pixel, _mm256_set1_epi32(0x00ff00ff));
        __m256i ag = _mm256_and_si256(_mm256_srli_epi16(pixel, 8), _mm256_set1_epi32(0x00ff00ff));

        rb = _mm256_madd_epi16(rb, m_weightsL);
        ag = _mm256_madd_epi16(ag, m_weightsH);
        pixel = _mm256_add_epi32(rb, ag);

        __m256i cmp0 = _mm256_cmpgt_epi32(pixel, m_min);
        __m256i cmp1 = _mm256_cmpgt_epi32(m_max, pixel);
        return _mm256_and_si256(cmp0, cmp1);
    }

private:
    const __m256i m_weightsL;
    const __m256i m_weightsH;
    const __m256i m_min;
    const __m256i m_max;
};




size_t filter_rgb32_brightness_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_AVX2 tester(
        weights,
        min_brightness, max_brightness
    );
    FilterImage_Rgb32_x64_AVX2<PixelTest_Rgb32Brightness_x64_AVX2> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_brightness_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_AVX2 tester(
        weights,
        min_brightness, max_brightness
    );
    ToBlackWhite_Rgb32_x64_AVX2<PixelTest_Rgb32Brightness_x64_AVX2> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}








}
}
#endif
