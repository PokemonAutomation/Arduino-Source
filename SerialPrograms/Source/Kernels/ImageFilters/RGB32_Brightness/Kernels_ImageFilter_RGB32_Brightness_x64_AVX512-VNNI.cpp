/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_19_IceLake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX512.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class PixelTest_Rgb32Brightness_x64_AVX512VNNI{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Brightness_x64_AVX512VNNI(
        Rgb32BrightnessWeights weights, int32_t min_sum, int32_t max_sum
    )
        : m_weights(_mm512_set1_epi32(weights.u32))
        , m_shift(_mm512_set1_epi32(-min_sum))
        , m_threshold(_mm512_set1_epi32(max_sum - min_sum))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __mmask16 test_word(__m512i pixel) const{
        pixel = _mm512_dpbusd_epi32(m_shift, pixel, m_weights);
        return _mm512_cmple_epu32_mask(pixel, m_threshold);
    }

private:
    const __m512i m_weights;
    const __m512i m_shift;
    const __m512i m_threshold;
};





size_t filter_rgb32_brightness_x64_AVX512VNNI(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_AVX512VNNI tester(
        weights,
        min_brightness, max_brightness
    );
    FilterImage_Rgb32_x64_AVX512<PixelTest_Rgb32Brightness_x64_AVX512VNNI> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_brightness_x64_AVX512VNNI(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_x64_AVX512VNNI tester(
        weights,
        min_brightness, max_brightness
    );
    ToBlackWhite_Rgb32_x64_AVX512<PixelTest_Rgb32Brightness_x64_AVX512VNNI> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}





















}
}
#endif
