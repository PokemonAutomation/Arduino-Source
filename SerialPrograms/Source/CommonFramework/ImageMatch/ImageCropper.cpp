/*  Image Cropper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/ImageTypes/ImageReference.h"
#include "ImageCropper.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace ImageMatch{



bool scan_row(const ImageViewRGB32& image, size_t row, const std::function<bool(QRgb)>& filter){
    for (size_t c = 0; c < image.width(); c++){
        QRgb pixel = image.pixel(c, row);
        if (filter(pixel)){
//            cout << "{" << c << "," << row << "}" << endl;
            return false;
        }
    }
    return true;
}
bool scan_col(const ImageViewRGB32& image, size_t col, const std::function<bool(QRgb)>& filter){
    for (size_t r = 0; r < image.height(); r++){
        QRgb pixel = image.pixel(col, r);
        if (filter(pixel)){
//            cout << "{" << col << "," << r << "}" << endl;
            return false;
        }
    }
    return true;
}

ImageViewRGB32 trim_image_alpha_ref(const ImageViewRGB32& image){
    size_t rs = 0;
    size_t re = image.height();
    size_t cs = 0;
    size_t ce = image.width();
    auto filter = [](QRgb pixel){
        return qAlpha(pixel) != 0;
    };
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return image.sub_image(cs, rs, ce - cs, re - rs);;
}
QImage trim_image_alpha(const ImageViewRGB32& image){
    return trim_image_alpha_ref(image).to_QImage_owning();
}
QImage trim_image_pixel_filter(const ImageViewRGB32& image, const std::function<bool(QRgb)>& filter){
    size_t rs = 0;
    size_t re = image.height();
    size_t cs = 0;
    size_t ce = image.width();
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return image.sub_image(cs, rs, ce - cs, re - rs).to_QImage_owning();
}

QRect enclosing_rectangle_with_pixel_filter(const ImageViewRGB32& image, const std::function<bool(QRgb)>& filter){
    size_t rs = 0;
    size_t re = image.height();
    size_t cs = 0;
    size_t ce = image.width();
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return QRect((int)cs, (int)rs, (int)(ce - cs), (int)(re - rs));
}






}
}
