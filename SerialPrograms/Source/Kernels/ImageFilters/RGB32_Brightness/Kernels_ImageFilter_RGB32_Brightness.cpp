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



size_t filter_rgb32_brightness_x64_AVX512VNNI(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t filter_rgb32_brightness_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t filter_rgb32_brightness_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t filter_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t filter_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_x64_19_IceLake
    if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
        return filter_rgb32_brightness_x64_AVX512VNNI(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            weights, min_brightness, max_brightness
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return filter_rgb32_brightness_x64_AVX2(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            weights, min_brightness, max_brightness
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return filter_rgb32_brightness_x64_SSE42(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            weights, min_brightness, max_brightness
        );
    }
#endif
    return filter_rgb32_brightness_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row,
        replacement, replace_color_within_range,
        weights, min_brightness, max_brightness
    );
}



size_t to_blackwhite_rgb32_brightness_x64_AVX512VNNI(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t to_blackwhite_rgb32_brightness_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t to_blackwhite_rgb32_brightness_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t to_blackwhite_rgb32_brightness_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
);
size_t to_blackwhite_rgb32_brightness(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    Rgb32BrightnessWeights weights,
    uint32_t min_brightness, uint32_t max_brightness
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_x64_19_IceLake
    if (CPU_CAPABILITY_CURRENT.OK_19_IceLake){
        return to_blackwhite_rgb32_brightness_x64_AVX512VNNI(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            in_range_black,
            weights, min_brightness, max_brightness
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return to_blackwhite_rgb32_brightness_x64_AVX2(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            in_range_black,
            weights, min_brightness, max_brightness
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return to_blackwhite_rgb32_brightness_x64_SSE42(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            in_range_black,
            weights, min_brightness, max_brightness
        );
    }
#endif
    return to_blackwhite_rgb32_brightness_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row,
        in_range_black,
        weights, min_brightness, max_brightness
    );
}




}
}
