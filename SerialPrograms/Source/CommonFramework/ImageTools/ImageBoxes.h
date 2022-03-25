/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_CommonFramework_ImageBoxes_H
#define PokemonAutomation_CommonFramework_ImageBoxes_H

#include <stddef.h>
#include "CommonFramework/ImageTypes/ImageReference.h"

class QImage;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}


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
    ImagePixelBox(const Kernels::Waterfill::WaterfillObject& object);

    pxint_t width() const{ return max_x - min_x; }
    pxint_t height() const{ return max_y - min_y; }
    size_t area() const{ return (size_t)width() * (size_t)height(); }

    void merge_with(const ImagePixelBox& box);
    size_t overlap_with(const ImagePixelBox& box) const;
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


//  Given an image, extract the request box from it.

//  Return a reference to the sub-region of the image.
ConstImageRef extract_box_reference(const ConstImageRef& image, const ImagePixelBox& box);
ConstImageRef extract_box_reference(const ConstImageRef& image, const ImageFloatBox& box);
ConstImageRef extract_box_reference(const QImage& image, const ImagePixelBox& box);
ConstImageRef extract_box_reference(const QImage& image, const ImageFloatBox& box);
ImageRef extract_box_reference(const ImageRef& image, const ImagePixelBox& box);
ImageRef extract_box_reference(const ImageRef& image, const ImageFloatBox& box);
ImageRef extract_box_reference(QImage& image, const ImagePixelBox& box);
ImageRef extract_box_reference(QImage& image, const ImageFloatBox& box);
ConstImageRef extract_box_reference(const ConstImageRef& image, const ImageFloatBox& box, int offset_x, int offset_y);

//  Deep copy the sub-region.
QImage extract_box_copy(const QImage& image, const ImagePixelBox& box);
QImage extract_box_copy(const QImage& image, const ImageFloatBox& box);

//  Given:
//      -   "inference_box" is a box within "original_image".
//      -   "box" is a box within "inference_box".
//
//  Translate "box" into a new box in the orignal image.
//
//  This is used for translating detection box within inference boxes back to
//  the parent so it can be displayed in a VideoOverlay.
ImageFloatBox translate_to_parent(
    const ConstImageRef& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
);


//  Given a ImagePixelBox within an image, get the ImageFloatBox for it.
ImagePixelBox floatbox_to_pixelbox(size_t width, size_t height, const ImageFloatBox& float_box);
ImageFloatBox pixelbox_to_floatbox(size_t width, size_t height, const ImagePixelBox& pixel_box);
ImageFloatBox pixelbox_to_floatbox(const QImage& image, const ImagePixelBox& pixel_box);


//  Given:
//      -   "inner" is a feature within an "object".
//      -   "inner_relative_to_image" is the box for the "inner" feature relative to the image.
//      -   "inner_relative_to_object" is the box for the "inner" feature within the object.
//
//  Return the enclosing box for the object in the original image.
//
//  This used by detection methods that detect a sub-feature within a larger
//  object. But then you need to expand out the object to match against a
//  template to confirm the detection.
ImagePixelBox extract_object_from_inner_feature(
    const ImagePixelBox& inner_relative_to_image,
    const ImageFloatBox& inner_relative_to_object
);
QImage extract_object_from_inner_feature(
    const QImage& image,
    const ImagePixelBox& inner_relative_to_image,
    const ImageFloatBox& inner_relative_to_object
);




}
#endif
