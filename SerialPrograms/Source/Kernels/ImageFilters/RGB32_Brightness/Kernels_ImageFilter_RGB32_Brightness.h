/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_RGB32_Brightness_H
#define PokemonAutomation_Kernels_ImageFilter_RGB32_Brightness_H

#include <stdint.h>
#include <cstddef>
#include "Common/Cpp/PixelRGB32.h"

namespace PokemonAutomation{
namespace Kernels{



union Rgb32BrightnessWeights{
    struct{
        int8_t blue;
        int8_t green;
        int8_t red;
        int8_t alpha;
    } parts;
    uint32_t u32;

    Rgb32BrightnessWeights(uint32_t value)
        : u32(value)
    {}
    Rgb32BrightnessWeights(uint8_t red, uint8_t green, uint8_t blue){
        parts.alpha = 0;
        parts.red = red;
        parts.green = green;
        parts.blue = blue;
    }
    Rgb32BrightnessWeights(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue){
        parts.alpha = alpha;
        parts.red = red;
        parts.green = green;
        parts.blue = blue;
    }
};



size_t filter_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);

size_t to_blackwhite_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);




}
}
#endif
