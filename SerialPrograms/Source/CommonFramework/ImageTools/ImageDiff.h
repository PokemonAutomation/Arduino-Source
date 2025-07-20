/*  Image Diff
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageDiff_H
#define PokemonAutomation_CommonFramework_ImageDiff_H

#include "CommonFramework/ImageTools/FloatPixel.h"

namespace PokemonAutomation{
    class ImageViewRGB32;
    class ImageRGB32;
namespace ImageMatch{


//  Compute average of all 4 components.
//    - The two images must be the same dimensions.
//    - The alpha channels of "reference" is used to indicate which parts to ignore.
//      0 means background. 255 means object. No other values are valid.
FloatPixel pixel_average(const ImageViewRGB32& image, const ImageViewRGB32& alpha_mask);


//  Multiply every pixel by "multiplier". Alpha channels are ignored.
void scale_brightness(ImageRGB32& image, const FloatPixel& multiplier);


//  Compute root-mean-square deviation of the two images.
//    - The two images must be the same dimensions.
//    - The alpha channels of "reference" must be 0 or 255. No other values are valid.
//
//      If (reference.alpha == 255)  Count the pixel.
//      If (reference.alpha ==   0)  Ignore the pixel and exclude from pixel count.
//
double pixel_RMSD(const ImageViewRGB32& reference, const ImageViewRGB32& image);


//  Compute root-mean-square deviation of the two images.
//    - The two images must be the same dimensions.
//    - The alpha channels of "reference" must be 0 or 255. No other values are valid.
//
//      If (reference.alpha == 255)  Count the pixel.
//      If (reference.alpha ==   0)  Replace reference pixel with "background".
//
double pixel_RMSD(const ImageViewRGB32& reference, const ImageViewRGB32& image, Color background);


//  Compute root-mean-square deviation of the two images.
//    - The two images must be the same dimensions.
//    - The alpha channels of both images must be 0 or 255. No other values are valid.
//
//      If (reference.alpha == 255 && image.alpha == 255)  Count the pixel.
//      If (reference.alpha == 255 && image.alpha ==   0)  Count the pixel as maximum possible distance. (255 for all RGB)
//      If (reference.alpha == 0   && image.alpha == 255)  Count the pixel as maximum possible distance. (255 for all RGB)
//      If (reference.alpha == 0   && image.alpha ==   0)  Ignore the pixel and exclude from pixel count.
//
double pixel_RMSD_masked(const ImageViewRGB32& reference, const ImageViewRGB32& image);



}
}
#endif
