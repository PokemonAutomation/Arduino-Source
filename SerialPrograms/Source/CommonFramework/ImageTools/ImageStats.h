/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageStats_H
#define PokemonAutomation_CommonFramework_ImageStats_H

#include "FloatPixel.h"

namespace PokemonAutomation{
    class ImageViewRGB32;

// Store basic stats of a group of pixels
struct ImageStats{
    // Average color among the pixels.
    FloatPixel average;
    // Stddev of the color for each color channel.
    // The smaller the stddev on one channel, the closer the pixel values are on this channel.
    FloatPixel stddev;
    // How many pixels in the group.
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
