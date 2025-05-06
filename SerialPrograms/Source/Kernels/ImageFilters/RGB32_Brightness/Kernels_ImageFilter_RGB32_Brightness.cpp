/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

namespace PokemonAutomation{
namespace Kernels{


size_t filter_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t filter_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t min_brightness, uint32_t max_brightness
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }

    return filter_rgb32_brightness_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row,
        replacement, replace_color_within_range,
        min_brightness, max_brightness
    );
}




size_t to_blackwhite_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t to_blackwhite_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t min_brightness, uint32_t max_brightness
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }

    return to_blackwhite_rgb32_brightness_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row,
        in_range_black,
        min_brightness, max_brightness
    );
}




}
}
