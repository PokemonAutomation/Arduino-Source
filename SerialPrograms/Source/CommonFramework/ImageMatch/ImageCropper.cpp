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



bool scan_row(const ConstImageRef& image, size_t row, const std::function<bool(QRgb)>& filter){
    for (size_t c = 0; c < image.width(); c++){
        QRgb pixel = image.pixel(c, row);
        if (filter(pixel)){
//            cout << "{" << c << "," << row << "}" << endl;
            return false;
        }
    }
    return true;
}
bool scan_col(const ConstImageRef& image, size_t col, const std::function<bool(QRgb)>& filter){
    for (size_t r = 0; r < image.height(); r++){
        QRgb pixel = image.pixel(col, r);
        if (filter(pixel)){
//            cout << "{" << col << "," << r << "}" << endl;
            return false;
        }
    }
    return true;
}

QImage trim_image_alpha(const QImage& image){
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid image format.");
    }

    int rs = 0;
    int re = image.height();
    int cs = 0;
    int ce = image.width();
    auto filter = [](QRgb pixel){
        return qAlpha(pixel) != 0;
    };
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return image.copy(cs, rs, ce - cs, re - rs);
}
QImage trim_image_pixel_filter(const QImage& image, const std::function<bool(QRgb)>& filter){
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid image format.");
    }

    int rs = 0;
    int re = image.height();
    int cs = 0;
    int ce = image.width();
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return image.copy(cs, rs, ce - cs, re - rs);
}
QRect enclosing_rectangle_with_pixel_filter(const QImage& image, const std::function<bool(QRgb)>& filter){
    if (image.format() != QImage::Format_RGB32 && image.format() != QImage::Format_ARGB32){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid image format.");
    }

    int rs = 0;
    int re = image.height();
    int cs = 0;
    int ce = image.width();
    while (rs < re && scan_row(image, rs    , filter)) rs++;
    while (re > rs && scan_row(image, re - 1, filter)) re--;
    while (cs < ce && scan_col(image, cs    , filter)) cs++;
    while (ce > cs && scan_col(image, ce - 1, filter)) ce--;
    return QRect(cs, rs, ce - cs, re - rs);
}


}
}
