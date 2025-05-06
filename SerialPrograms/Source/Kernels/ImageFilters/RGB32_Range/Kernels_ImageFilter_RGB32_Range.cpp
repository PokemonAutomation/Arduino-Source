/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_ImageFilter_RGB32_Range.h"

namespace PokemonAutomation{
namespace Kernels{




size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
);
size_t filter_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
);
size_t filter_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
);
size_t filter_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
);
size_t filter_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
);
size_t filter_rgb32_range(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool replace_color_within_range
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return filter_rgb32_range_x64_AVX512(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row, mins, maxs, replacement, replace_color_within_range
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return filter_rgb32_range_x64_AVX2(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row, mins, maxs, replacement, replace_color_within_range
        );
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return filter_rgb32_range_x64_SSE42(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row, mins, maxs, replacement, replace_color_within_range
        );
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return filter_rgb32_range_arm64_NEON(
            in, in_bytes_per_row, width, height,
            out, out_bytes_per_row, mins, maxs, replacement, replace_color_within_range
        );
    }
#endif
    return filter_rgb32_range_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row, mins, maxs, replacement, replace_color_within_range
    );
}



void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);
void filter_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);
void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);
void filter_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);
void filter_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);
void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        filter_rgb32_range_x64_AVX512(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        filter_rgb32_range_x64_AVX2(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        filter_rgb32_range_x64_SSE42(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        filter_rgb32_range_arm64_NEON(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
    filter_rgb32_range_Default(image, bytes_per_row, width, height, filter, filter_count);
}








size_t to_blackwhite_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);
size_t to_blackwhite_rgb32_range_x64_SSE42(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);
size_t to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);
size_t to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);
size_t to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);
size_t to_blackwhite_rgb32_range(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return to_blackwhite_rgb32_range_x64_AVX512(in, in_bytes_per_row, width, height, out, out_bytes_per_row, mins, maxs, in_range_black);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return to_blackwhite_rgb32_range_x64_AVX2(in, in_bytes_per_row, width, height, out, out_bytes_per_row, mins, maxs, in_range_black);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return to_blackwhite_rgb32_range_x64_SSE42(in, in_bytes_per_row, width, height, out, out_bytes_per_row, mins, maxs, in_range_black);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return to_blackwhite_rgb32_range_arm64_NEON(in, in_bytes_per_row, width, height, out, out_bytes_per_row, mins, maxs, in_range_black);
    }
#endif
    return to_blackwhite_rgb32_range_Default(in, in_bytes_per_row, width, height, out, out_bytes_per_row, mins, maxs, in_range_black);
}



void to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);
void to_blackwhite_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);
void to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);
void to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);
void to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);
void to_blackwhite_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
#ifdef PA_AutoDispatch_x64_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        return to_blackwhite_rgb32_range_x64_AVX512(image, bytes_per_row, width, height, filter, filter_count);
    }
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        return to_blackwhite_rgb32_range_x64_AVX2(image, bytes_per_row, width, height, filter, filter_count);
    }
#endif
#ifdef PA_AutoDispatch_x64_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        return to_blackwhite_rgb32_range_x64_SSE42(image, bytes_per_row, width, height, filter, filter_count);
    }
#endif
#ifdef PA_AutoDispatch_arm64_20_M1
    if (CPU_CAPABILITY_CURRENT.OK_M1){
        return to_blackwhite_rgb32_range_arm64_NEON(image, bytes_per_row, width, height, filter, filter_count);
    }
#endif
    return to_blackwhite_rgb32_range_Default(image, bytes_per_row, width, height, filter, filter_count);
}




}
}
