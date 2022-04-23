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

void filter2_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
);
void filter2_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
);
void filter2_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
);
void filter2_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
);

void filter4_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    size_t& count2, uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    size_t& count3, uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
);
void filter4_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    size_t& count2, uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    size_t& count3, uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
);
void filter4_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    size_t& count2, uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    size_t& count3, uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
);
void filter4_rgb32_range_x64_AVX512(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    size_t& count2, uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    size_t& count3, uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
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
void filter2_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        filter2_rgb32_range_x64_AVX512(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        filter2_rgb32_range_x64_AVX2(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        filter2_rgb32_range_x64_SSE42(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1
        );
        return;
    }
#endif
    filter2_rgb32_range_Default(
        image, bytes_per_row, width, height,
        count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
        count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1
    );
}
void filter4_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    size_t& count1, uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    size_t& count2, uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    size_t& count3, uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
#ifdef PA_AutoDispatch_17_Skylake
    if (CPU_CAPABILITY_CURRENT.OK_17_Skylake){
        filter4_rgb32_range_x64_AVX512(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1,
            count2, out2, bytes_per_row2, mins2, maxs2, replacement2, invert2,
            count3, out3, bytes_per_row3, mins3, maxs3, replacement3, invert3
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_13_Haswell
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        filter4_rgb32_range_x64_AVX2(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1,
            count2, out2, bytes_per_row2, mins2, maxs2, replacement2, invert2,
            count3, out3, bytes_per_row3, mins3, maxs3, replacement3, invert3
        );
        return;
    }
#endif
#ifdef PA_AutoDispatch_08_Nehalem
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        filter4_rgb32_range_x64_SSE42(
            image, bytes_per_row, width, height,
            count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
            count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1,
            count2, out2, bytes_per_row2, mins2, maxs2, replacement2, invert2,
            count3, out3, bytes_per_row3, mins3, maxs3, replacement3, invert3
        );
        return;
    }
#endif
    filter4_rgb32_range_Default(
        image, bytes_per_row, width, height,
        count0, out0, bytes_per_row0, mins0, maxs0, replacement0, invert0,
        count1, out1, bytes_per_row1, mins1, maxs1, replacement1, invert1,
        count2, out2, bytes_per_row2, mins2, maxs2, replacement2, invert2,
        count3, out3, bytes_per_row3, mins3, maxs3, replacement3, invert3
    );
}





}
}
