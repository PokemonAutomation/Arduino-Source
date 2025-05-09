/*  Image Filters Basic
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/CpuId/CpuId.h"
//#include "Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_Basic.h"

namespace PokemonAutomation{
namespace Kernels{





size_t filter_green_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, uint8_t rgb_gap
);


size_t filter_green(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, uint8_t rgb_gap
){
    if (width * height > 0xffffffff){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Image is too large. more than 2^32 pixels.");
    }
    return filter_green_Default(
        in, in_bytes_per_row, width, height,
        out, out_bytes_per_row, replacement, rgb_gap
    );
}



}
}
