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

ImageStats image_stats(const QImage& image);
ImageStats image_border_stats(const QImage& image);



}
#endif
