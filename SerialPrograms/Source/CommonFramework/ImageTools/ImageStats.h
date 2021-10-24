/*  Image Stats
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageStats_H
#define PokemonAutomation_CommonFramework_ImageStats_H

#include <QImage>
#include "FloatPixel.h"

namespace PokemonAutomation{


struct ImageStats{
    FloatPixel average;
    FloatPixel stddev;
};


//  Pixels with alpha < 128 are ignored.
FloatPixel image_average(const QImage& image);
FloatPixel image_stddev(const QImage& image);
ImageStats image_stats(const QImage& image);


ImageStats image_border_stats(const QImage& image);



}
#endif
