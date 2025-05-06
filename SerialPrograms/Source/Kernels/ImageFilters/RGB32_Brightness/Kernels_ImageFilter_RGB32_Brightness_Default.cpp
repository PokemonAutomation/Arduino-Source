/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbBrightness_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilter_RgbBrightness_Default(
        uint32_t min_brightness, uint32_t max_brightness,
        uint32_t replacement_color, bool replace_color_within_range
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
    ImageFilter_RgbBrightness_Default filter(replacement, replace_color_within_range, min_brightness, max_brightness);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}





}
}
