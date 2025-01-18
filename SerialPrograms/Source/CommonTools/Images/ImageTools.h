/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageTools_H
#define PokemonAutomation_CommonFramework_ImageTools_H


namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;


//  Deprecated
ImageRGB32 image_diff_greyscale(const ImageViewRGB32& x, const ImageViewRGB32& y);





}
#endif

