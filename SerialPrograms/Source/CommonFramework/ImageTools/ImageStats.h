/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageStats_H
#define PokemonAutomation_CommonFramework_ImageStats_H

#include "CommonFramework/ImageTypes/ImageReference.h"
#include "FloatPixel.h"

class QImage;

namespace PokemonAutomation{


struct ImageStats{
    FloatPixel average;
    FloatPixel stddev;
};


//  Pixels with alpha < 128 are ignored.
FloatPixel image_average(const ConstImageRef& image);
FloatPixel image_stddev(const ConstImageRef& image);
ImageStats image_stats(const ConstImageRef& image);


ImageStats image_border_stats(const ConstImageRef& image);



}
#endif
