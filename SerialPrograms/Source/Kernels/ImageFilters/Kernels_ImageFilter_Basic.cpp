/*  Image Filters Basic
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId.h"
#include "Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_Basic.h"

namespace PokemonAutomation{
namespace Kernels{


void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
);
void filter_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
);
void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
);
void filter_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
);


void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        filter_rgb32_range_x64_AVX512(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        filter_rgb32_range_x64_AVX2(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        filter_rgb32_range_x64_SSE42(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0
        );
        return;
    }
#endif
    filter_rgb32_range_Default(
        image, bytes_per_row, width, height,
        count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0
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


void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        filter_rgb32_range_x64_AVX512(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        filter_rgb32_range_x64_AVX2(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        filter_rgb32_range_x64_SSE42(image, bytes_per_row, width, height, filter, filter_count);
        return;
    }
#endif
    filter_rgb32_range_Default(image, bytes_per_row, width, height, filter, filter_count);
}





}
}
