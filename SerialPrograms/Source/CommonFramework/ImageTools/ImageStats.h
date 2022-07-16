/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageStats_H
#define PokemonAutomation_CommonFramework_ImageStats_H

#include "FloatPixel.h"

namespace PokemonAutomation{
    class ImageViewRGB32;


struct ImageStats{
    FloatPixel average;
    FloatPixel stddev;
    uint64_t count;
    ImageStats() : count(0) {}
    ImageStats(FloatPixel a, FloatPixel s, uint64_t c) : average(a), stddev(s), count(c) {}
};


//  Pixels with alpha < 128 are ignored.
FloatPixel image_average(const ImageViewRGB32& image);
FloatPixel image_stddev(const ImageViewRGB32& image);
ImageStats image_stats(const ImageViewRGB32& image);


ImageStats image_border_stats(const ImageViewRGB32& image);



}
#endif
