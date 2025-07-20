/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

namespace PokemonAutomation{
namespace Kernels{


size_t filter_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t filter_rgb32_euclidean_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t filter_rgb32_euclidean_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t filter_rgb32_euclidean_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t filter_rgb32_euclidean_ARM64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
);
size_t filter_rgb32_euclidean(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return filter_rgb32_euclidean_x64_AVX512(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            expected, max_euclidean_distance
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return filter_rgb32_euclidean_x64_AVX2(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            expected, max_euclidean_distance
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return filter_rgb32_euclidean_x64_SSE42(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            expected, max_euclidean_distance
        );
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return filter_rgb32_euclidean_ARM64_NEON(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row,
            replacement, replace_color_within_range,
            expected, max_euclidean_distance
        );
    }
#endif
    return filter_rgb32_euclidean_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row,
            replacement, replace_color_within_range,
            expected, max_euclidean_distance
    );
}







}
}
