/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <cmath>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ImageTools.h"

namespace PokemonAutomation{



ImageRGB32 image_diff_greyscale(const ImageViewRGB32& x, const ImageViewRGB32& y){
    if (!x || !y){
        return ImageRGB32();
    }
    if (x.width() != y.width()){
        return ImageRGB32();
    }
    if (x.height() != y.height()){
        return ImageRGB32();
    }

    ImageRGB32 image(x.width(), x.height());
    size_t width = x.width();
    size_t height = x.height();
    for (size_t r = 0; r < height; r++){
        for (size_t c = 0; c < width; c++){
            uint32_t px = x.pixel(c, r);
            uint32_t py = y.pixel(c, r);
            if (qAlpha(px) == 0 || qAlpha(py) == 0){
                image.get_pixel(c, r) = 0xff000000;
            }else{
                double distance = euclidean_distance(px, py);
                distance *= 0.57735026918962576451;  //  1 / sqrt(3)
                uint32_t dist_int = std::min<uint32_t>((uint32_t)distance, 255);
                image.get_pixel(c, r) = 0xff000000 + dist_int * 0x010101;
            }
        }
    }
    return image;
}












}

