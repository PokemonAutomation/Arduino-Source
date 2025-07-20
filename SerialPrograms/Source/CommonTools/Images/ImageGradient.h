/*  Image Gradient
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Header for various image gradient related functions, mostly for visual inferences
 */

#ifndef PokemonAutomation_CommonTools_ImageGradient_H
#define PokemonAutomation_CommonTools_ImageGradient_H

#include <cstddef>
#include <vector>
#include "Common/Cpp/Color.h"

namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;

// Used to detect a horizontal border line across an image.
// The border is defined as:
// If `dark_top` is true, the upper side of the border line is darker than the lower side.
// If `dark_top` is false, the lower side of the border line is darker than the upper side.
// It returns how many pixels on the border that are darker by the other side by at least `threshold`
// If it returns the same number as `image.width()`, then there is a clear horizontal border across the image.
//
// The image is supposed to have a very small height, as it only holds a horizontal border line.
size_t count_horizontal_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_top);

// Used to detect a vertical border line across an image.
// The border is defined as:
// If `dark_left` is true, the left side of the border line is darker than the right side.
// If `dark_left` is false, the right side of the border line is darker than the left side.
// It returns how many pixels on the border that are darker by the other side by at least `threshold`
// If it returns the same number as `image.height()`, then there is a clear vertical border across the image.
//
// The image is supposed to have a very small width, as it only holds a vertical border line.
size_t count_vertical_translucent_border_pixels(const ImageViewRGB32& image, const Color& threshold, bool dark_left);

}
#endif
