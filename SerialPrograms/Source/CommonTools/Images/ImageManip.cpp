/*  Image Manipulation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string.h>
#include "ImageManip.h"

namespace PokemonAutomation{


ImageRGB32 pad_image(const ImageViewRGB32& image, size_t pixels_on_each_side, uint32_t pixel){
    size_t width = image.width();
    size_t height = image.height();
    ImageRGB32 ret(image.width() + 2*pixels_on_each_side, image.height() + 2*pixels_on_each_side);
    ret.fill(pixel);
    for (size_t r = 0; r < height; r++){
        size_t out_row = r + pixels_on_each_side;
        uint32_t* out = (uint32_t*)((char*)ret.data() + ret.bytes_per_row() * out_row) + pixels_on_each_side;
        const uint32_t* in = (const uint32_t*)((char*)image.data() + image.bytes_per_row() * r);
        memcpy(out, in, width * sizeof(uint32_t));
    }
    return ret;
}


}
