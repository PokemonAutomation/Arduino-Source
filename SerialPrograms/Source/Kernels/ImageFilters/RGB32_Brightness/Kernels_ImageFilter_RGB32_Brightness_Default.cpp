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





#if 0
class ImageFilter_RgbBrightness_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilter_RgbBrightness_Default(
        uint32_t replacement_color, bool replace_color_within_range,
        uint32_t min_brightness, uint32_t max_brightness
    )
        : m_replacement_color(replacement_color)
        , m_replace_color_within_range(replace_color_within_range ? 1 : 0)
        , m_min_brightness(min_brightness)
        , m_max_brightness(max_brightness)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint32_t brightness = 0;
        brightness += ((pixel >>  0) & 0x000000ff);
        brightness += ((pixel >>  8) & 0x000000ff);
        brightness += ((pixel >> 16) & 0x000000ff);
        uint32_t ret = m_min_brightness <= brightness && brightness <= m_max_brightness;
        ret ^= m_replace_color_within_range;
        out[0] = ret ? pixel : m_replacement_color;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_replacement_color;
    const uint32_t m_replace_color_within_range;
    uint32_t m_min_brightness;
    uint32_t m_max_brightness;
    size_t m_count;
};
size_t filter_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t min_brightness, uint32_t max_brightness
){
    ImageFilter_RgbBrightness_Default filter(
        replacement, replace_color_within_range,
        min_brightness, max_brightness
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
#endif








size_t to_blackwhite_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t min_brightness, uint32_t max_brightness
){
    PixelTest_Rgb32Brightness_Default tester(
        Rgb32BrightnessWeights(0x00010101),
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
