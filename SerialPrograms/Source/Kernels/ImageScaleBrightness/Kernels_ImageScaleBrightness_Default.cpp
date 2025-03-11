/*  Scale Brightness (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <algorithm>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


PA_FORCE_INLINE void scale_brightness_Default(
    size_t width, uint32_t* image,
    float scaleR, float scaleG, float scaleB
){
    for (size_t c = 0; c < width; c++){
        uint32_t pixel = image[c];
        float r = (float)((pixel >> 16) & 0x000000ff);
        float g = (float)((pixel >> 8) & 0x000000ff);
        float b = (float)(pixel & 0x000000ff);
        r *= scaleR;
        g *= scaleG;
        b *= scaleB;

        uint32_t r_u32 = std::min((uint32_t)r, (uint32_t)255);
        uint32_t g_u32 = std::min((uint32_t)g, (uint32_t)255);
        uint32_t b_u32 = std::min((uint32_t)b, (uint32_t)255);

        pixel &= 0xff000000;
        pixel |= r_u32 << 16;
        pixel |= g_u32 << 8;
        pixel |= b_u32;

        image[c] = pixel;
    }
}
void scale_brightness_Default(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
    if (width == 0 || height == 0){
        return;
    }
    scaleR = std::max(scaleR, 0.0f);
    scaleG = std::max(scaleG, 0.0f);
    scaleB = std::max(scaleB, 0.0f);
    for (uint16_t r = 0; r < height; r++){
        scale_brightness_Default(width, image, scaleR, scaleG, scaleB);
        image = (uint32_t*)((char*)image + bytes_per_row);
    }
}




}
}
