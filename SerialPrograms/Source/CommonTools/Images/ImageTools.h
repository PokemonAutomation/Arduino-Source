/*  Image Tools
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CommonTools_ImageTools_H
#define PokemonAutomation_CommonTools_ImageTools_H


namespace PokemonAutomation{

class ImageViewRGB32;
class ImageRGB32;
struct ImagePixelBox;
struct ImageFloatBox;


//  Deprecated
ImageRGB32 image_diff_greyscale(const ImageViewRGB32& x, const ImageViewRGB32& y);


ImagePixelBox find_contents_pixel_box(const ImageViewRGB32& image);
ImageFloatBox find_contents_float_box(const ImageViewRGB32& image);





}
#endif

