/*  Scale Brightness
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "CommonFramework/ImageTools/FloatPixel.h"
#include "ScaleBrightness.h"

namespace PokemonAutomation{
namespace ImageMatch{


#if 0
void scale_size_and_brightness(const QImage& reference, QImage& image){
    if (image.size() != reference.size()){
        image = image.scaled(reference.size());
    }

    FloatPixel image_brightness = pixel_average(image, m_image);
    FloatPixel scale = image_brightness / m_stats.average;



}
#endif




}
}
