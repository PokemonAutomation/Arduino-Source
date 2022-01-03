/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageBoxes_H
#define PokemonAutomation_CommonFramework_ImageBoxes_H

#include <stddef.h>

class QImage;

namespace PokemonAutomation{


using pxint_t = int;


struct ImagePixelBox{
    pxint_t min_x;
    pxint_t min_y;
    pxint_t max_x;  //  One past the end.
    pxint_t max_y;  //  One past the end.

    ImagePixelBox() = default;
    ImagePixelBox(pxint_t p_min_x, pxint_t p_min_y, pxint_t p_max_x, pxint_t p_max_y)
        : min_x(p_min_x), min_y(p_min_y), max_x(p_max_x), max_y(p_max_y)
    {}
    ImagePixelBox(size_t p_min_x, size_t p_min_y, size_t p_max_x, size_t p_max_y);

    pxint_t width() const{ return max_x - min_x; }
    pxint_t height() const{ return max_y - min_y; }
    size_t area() const{ return (size_t)width() * (size_t)height(); }
};


struct ImageFloatBox{
    double x;
    double y;
    double width;
    double height;

    ImageFloatBox()
        : x(0), y(0)
        , width(0), height(0)
    {}
    ImageFloatBox(
        double p_x, double p_y,
        double p_width, double p_height
    )
        : x(p_x), y(p_y)
        , width(p_width), height(p_height)
    {}
};


QImage extract_box(const QImage& image, const ImagePixelBox& box);
QImage extract_box(const QImage& image, const ImageFloatBox& box);
QImage extract_box(const QImage& image, const ImageFloatBox& box, int offset_x, int offset_y);


ImageFloatBox translate_to_parent(
    const QImage& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
);





}
#endif
