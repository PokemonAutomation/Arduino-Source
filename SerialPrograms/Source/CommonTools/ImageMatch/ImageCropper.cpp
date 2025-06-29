/*  Image Cropper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "ImageCropper.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{


// Scan one row to find if all pixels in the row are from background
bool is_background_row(const ImageViewRGB32& image, size_t row, const std::function<bool(Color)>& is_foreground){
    for (size_t c = 0; c < image.width(); c++){
        Color pixel(image.pixel(c, row));
        if (is_foreground(pixel)){
//            cout << "{" << c << "," << row << "}" << endl;
            return false;
        }
    }
    return true;
}

// Scan one col to find if all pixels in the col are from background
bool is_background_col(const ImageViewRGB32& image, size_t col, const std::function<bool(Color)>& is_foreground){
    for (size_t r = 0; r < image.height(); r++){
        Color pixel(image.pixel(col, r));
        if (is_foreground(pixel)){
//            cout << "{" << col << "," << r << "}" << endl;
            return false;
        }
    }
    return true;
}

ImageViewRGB32 trim_image_alpha(const ImageViewRGB32& image, uint8_t alpha_threshold){
    auto is_foreground = [=](Color pixel){
        return pixel.alpha() >= alpha_threshold;
    };
    const auto box = enclosing_rectangle_with_pixel_filter(image, is_foreground);

    return extract_box_reference(image, box);
}

ImagePixelBox enclosing_rectangle_with_pixel_filter(const ImageViewRGB32& image, const std::function<bool(Color)>& is_foreground){
    size_t rs = 0;
    size_t re = image.height();
    size_t cs = 0;
    size_t ce = image.width();

//    cout << "Before: rs = " << rs << ", re = " << re << ", cs = " << cs << ", ce = " << ce << endl;

    // If we cannot find a foreground (aka, alpha != 0) pixel on one row or column, we trim it, until hitting a foreground pixel.
    while (rs < re && is_background_row(image, rs    , is_foreground)) rs++;
    while (re > rs && is_background_row(image, re - 1, is_foreground)) re--;
    while (cs < ce && is_background_col(image, cs    , is_foreground)) cs++;
    while (ce > cs && is_background_col(image, ce - 1, is_foreground)) ce--;

//    cout << "After:  rs = " << rs << ", re = " << re << ", cs = " << cs << ", ce = " << ce << endl;

    return ImagePixelBox(cs, rs, ce, re);
}






}
}
