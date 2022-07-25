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
    class ImageViewRGB32;
    class ImageRGB32;
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
} // end namespace Waterfill
} // end namespace Kernels


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

    pxint_t center_x() const{ return (min_x + max_x)/2; }
    pxint_t center_y() const{ return (min_y + max_y)/2; }

    // Create a box covering both `this` box and the parameter `box` passed in.
    // If the parameter `box` has 0 area, do no change.
    // If `this` box has 0 area, `this` becomes the parameter `box`.
    void merge_with(const ImagePixelBox& box);

    // Return whether two boxes overlap. Boxes touching each other does not count as overlap.
    bool overlap(const ImagePixelBox& box) const;

    // Return the overlapping area of `this` box and the parameter `box` passed in.
    size_t overlap_with(const ImagePixelBox& box) const;

    // Whether a point (x, y) is inside the box. Points on the border of the box does not
    // count as inside.
    bool inside(pxint_t x, pxint_t y) const;

    // clip the box to be within the image size.
    void clip(size_t image_width, size_t image_height);
    
    // The distance to another box on x axis. If two boxes overlap, the distance is 0.
    size_t distance_x(const ImagePixelBox& box) const;
    // The distance to another box on y axis. If two boxes overlap, the distance is 0.
    size_t distance_y(const ImagePixelBox& box) const;
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
ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImagePixelBox& box);
ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImageFloatBox& box);
ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImageFloatBox& box, ptrdiff_t offset_x, ptrdiff_t offset_y);

//  Deprecated
ImageRef extract_box_reference(QImage& image, const ImagePixelBox& box);
ImageRef extract_box_reference(QImage& image, const ImageFloatBox& box);



//  Given:
//      -   "inference_box" is a box within "original_image".
//      -   "box" is a box within "inference_box".
//
//  Translate "box" into a new box in the orignal image.
//
//  This is used for translating detection box within inference boxes back to
//  the parent so it can be displayed in a VideoOverlay.
ImageFloatBox translate_to_parent(
    const ImageViewRGB32& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
);


//  Given a ImagePixelBox within an image, get the ImageFloatBox for it.
ImagePixelBox floatbox_to_pixelbox(size_t width, size_t height, const ImageFloatBox& float_box);
ImageFloatBox pixelbox_to_floatbox(size_t width, size_t height, const ImagePixelBox& pixel_box);
ImageFloatBox pixelbox_to_floatbox(const ImageViewRGB32& image, const ImagePixelBox& pixel_box);


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

// Draw a box on the image. Used for debugging purposes:
// save inference results to an image on the disk.
// color: the color of the pixel. See Common/Cpp/Color.h:Color on the color format.
// thickness: thickness (in unit of pixels) of the box border.
void draw_box(ImageRGB32& image, const ImagePixelBox& pixel_box, uint32_t color, size_t thickness = 1);



}
#endif
