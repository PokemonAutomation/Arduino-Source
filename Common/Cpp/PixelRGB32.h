/*  Pixel: RGB32
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Perform a filter over an image and replace pixels that match the filter.
 *
 */

#ifndef PokemonAutomation_Pixel_RGB32_H
#define PokemonAutomation_Pixel_RGB32_H

#include <stdint.h>

namespace PokemonAutomation{





union PixelRGB32{
    struct{
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } parts;
    uint32_t u32;

    PixelRGB32(uint32_t value)
        : u32(value)
    {}
    PixelRGB32(uint8_t red, uint8_t green, uint8_t blue){
        parts.alpha = 0xff;
        parts.red = red;
        parts.green = green;
        parts.blue = blue;
    }
    PixelRGB32(uint8_t alpha, uint8_t red, uint8_t green, uint8_t blue){
        parts.alpha = alpha;
        parts.red = red;
        parts.green = green;
        parts.blue = blue;
    }
};





}
#endif
