/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageTools_H
#define PokemonAutomation_CommonFramework_ImageTools_H

#include <ostream>
#include <QImage>
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/FillGeometry.h"

namespace PokemonAutomation{



QImage image_diff_greyscale(const QImage& x, const QImage& y);





}
#endif

