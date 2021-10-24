/*  Scale Brightness (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "Common/Compiler.h"
#include "Kernels_ImageScaleBrightness_Default.h"

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

        r = std::max(r, 0.f);
        g = std::max(g, 0.f);
        b = std::max(b, 0.f);
        r = std::min(r, 255.f);
        g = std::min(g, 255.f);
        b = std::min(b, 255.f);

        pixel &= 0xff000000;
        pixel |= (uint32_t)r << 16;
        pixel |= (uint32_t)g << 8;
        pixel |= (uint32_t)b;

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
    for (uint16_t r = 0; r < height; r++){
        scale_brightness_Default(width, image, scaleR, scaleG, scaleB);
        image = (uint32_t*)((char*)image + bytes_per_row);
    }
}




}
}
