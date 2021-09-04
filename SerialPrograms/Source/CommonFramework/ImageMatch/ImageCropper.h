/*  Image Cropper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageCropper_H
#define PokemonAutomation_CommonFramework_ImageCropper_H

#include <functional>
#include <QImage>

namespace PokemonAutomation{
namespace ImageMatch{


//  Shrink image so that it perfectly fits the object.
//  The alpha channel is used to determine what is object vs. background.
QImage trim_image_alpha(const QImage& image);


//  Shrink image so that it perfectly fits the object.
//  The filter returns true if it is part of the object.
QImage trim_image_pixel_filter(const QImage& image, const std::function<bool(QRgb)>& filter);


//  Filter the image using the specified pixel filter.
//  Return the enclosing rectangle.
QRect enclosing_rectangle_with_pixel_filter(const QImage& image, const std::function<bool(QRgb)>& filter);


}
}
#endif
