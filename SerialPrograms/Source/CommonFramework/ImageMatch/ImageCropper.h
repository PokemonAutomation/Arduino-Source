/*  Image Cropper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageCropper_H
#define PokemonAutomation_CommonFramework_ImageCropper_H

#include <functional>
#include "Common/Cpp/Color.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"

namespace PokemonAutomation{
namespace ImageMatch{


//  Shrink image so that it perfectly fits the object.
//  The alpha channel is used to determine what is object vs. background.
//  background is defined as alpha == 0.
ImageViewRGB32 trim_image_alpha(const ImageViewRGB32& image);


//  Filter out object from background.
//  The pixels of the object are defined as is_object(pixel_color) == true.
//  Return the rectangle enclosing the object.
ImagePixelBox enclosing_rectangle_with_pixel_filter(const ImageViewRGB32& image, const std::function<bool(Color)>& is_object);



}
}
#endif
