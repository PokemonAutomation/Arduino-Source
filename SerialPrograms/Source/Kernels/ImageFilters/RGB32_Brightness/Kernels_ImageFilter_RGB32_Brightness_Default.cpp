/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_Default.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{


class PixelTest_Rgb32Brightness_Default{
public:
    PA_FORCE_INLINE PixelTest_Rgb32Brightness_Default(
        Rgb32BrightnessWeights weights, int32_t min_sum, int32_t max_sum
    )
        : m_weight_a(weights.parts.alpha)
        , m_weight_r(weights.parts.red)
        , m_weight_g(weights.parts.green)
        , m_weight_b(weights.parts.blue)
        , m_shift(min_sum)
        , m_threshold(max_sum - min_sum)
    {}
    PA_FORCE_INLINE bool test_word(uint32_t pixel) const{
        uint32_t brightness = 0;
        brightness += ((pixel >> 24) & 0x000000ff) * m_weight_a;
        brightness += ((pixel >> 16) & 0x000000ff) * m_weight_r;
        brightness += ((pixel >>  8) & 0x000000ff) * m_weight_g;
        brightness += ((pixel >>  0) & 0x000000ff) * m_weight_b;
        return brightness - m_shift <= m_threshold;
    }

private:
    int32_t m_weight_a;
    int32_t m_weight_r;
    int32_t m_weight_g;
    int32_t m_weight_b;
    uint32_t m_shift;
    uint32_t m_threshold;
};







size_t filter_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_Default tester(
        weights,
        min_brightness, max_brightness
    );
    FilterImage_Rgb32_Default<PixelTest_Rgb32Brightness_Default> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_Default tester(
        weights,
        min_brightness, max_brightness
    );
    ToBlackWhite_Rgb32_Default<PixelTest_Rgb32Brightness_Default> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
