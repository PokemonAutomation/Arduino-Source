/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include <QImage>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "ImageBoxes.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


ImagePixelBox::ImagePixelBox(size_t p_min_x, size_t p_min_y, size_t p_max_x, size_t p_max_y)
    : min_x((pxint_t)p_min_x), min_y((pxint_t)p_min_y)
    , max_x((pxint_t)p_max_x), max_y((pxint_t)p_max_y)
{
    if (min_x != (int64_t)p_min_x || max_x != (int64_t)p_max_x){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Pixel Overflow: x = (" + std::to_string(p_min_x) + "," + std::to_string(p_max_x) + ")"
        );
    }
    if (min_y != (int64_t)p_min_y || max_y != (int64_t)p_max_y){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Pixel Overflow: y = (" + std::to_string(p_min_y) + "," + std::to_string(p_max_y) + ")"
        );
    }
}
ImagePixelBox::ImagePixelBox(const Kernels::Waterfill::WaterfillObject& object)
    : ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
{}


void ImagePixelBox::merge_with(const ImagePixelBox& box){
    if (box.area() == 0){
        return;
    }
    if (this->area() == 0){
        *this = box;
    }
    min_x = std::min(min_x, box.min_x);
    min_y = std::min(min_y, box.min_y);
    max_x = std::max(max_x, box.max_x);
    max_y = std::max(max_y, box.max_y);
}
size_t ImagePixelBox::overlap_with(const ImagePixelBox& box) const{
    pxint_t min_x = std::max(this->min_x, box.min_x);
    pxint_t max_x = std::min(this->max_x, box.max_x);
    if (min_x >= max_x){
        return 0;
    }
    pxint_t min_y = std::max(this->min_y, box.min_y);
    pxint_t max_y = std::min(this->max_y, box.max_y);
    if (min_y >= max_y){
        return 0;
    }
    return (size_t)(max_x - min_x) * (size_t)(max_y - min_y);
}




ConstImageRef extract_box_reference(const ConstImageRef& image, const ImagePixelBox& box){
    return image.sub_image(box.min_x, box.min_y, box.width(), box.height());
}
ConstImageRef extract_box_reference(const ConstImageRef& image, const ImageFloatBox& box){
    size_t min_x = (size_t)(image.width() * box.x + 0.5);
    size_t min_y = (size_t)(image.height() * box.y + 0.5);
    size_t width = (size_t)(image.width() * box.width + 0.5);
    size_t height = (size_t)(image.height() * box.height + 0.5);
    return image.sub_image(min_x, min_y, width, height);
}
ImageRef extract_box_reference(const ImageRef& image, const ImagePixelBox& box){
    return image.sub_image(box.min_x, box.min_y, box.width(), box.height());
}
ImageRef extract_box_reference(const ImageRef& image, const ImageFloatBox& box){
    size_t min_x = (size_t)(image.width() * box.x + 0.5);
    size_t min_y = (size_t)(image.height() * box.y + 0.5);
    size_t width = (size_t)(image.width() * box.width + 0.5);
    size_t height = (size_t)(image.height() * box.height + 0.5);
    return image.sub_image(min_x, min_y, width, height);
}

ConstImageRef extract_box_reference(const QImage& image, const ImagePixelBox& box){
    return extract_box_reference(ConstImageRef(image), box);
}
ConstImageRef extract_box_reference(const QImage& image, const ImageFloatBox& box){
    return extract_box_reference(ConstImageRef(image), box);
}
ImageRef extract_box_reference(QImage& image, const ImagePixelBox& box){
    return extract_box_reference(ImageRef(image), box);
}
ImageRef extract_box_reference(QImage& image, const ImageFloatBox& box){
    return extract_box_reference(ImageRef(image), box);
}
ConstImageRef extract_box_reference(const ConstImageRef& image, const ImageFloatBox& box, int offset_x, int offset_y){
    ptrdiff_t min_x = (ptrdiff_t)(image.width() * box.x + 0.5) + offset_x;
    ptrdiff_t min_y = (ptrdiff_t)(image.height() * box.y + 0.5) + offset_y;
    ptrdiff_t width = (ptrdiff_t)(image.width() * box.width + 0.5);
    ptrdiff_t height = (ptrdiff_t)(image.height() * box.height + 0.5);

    if (min_x < 0){
        width += min_x;
        min_x = 0;
        width = std::max<ptrdiff_t>(width, 0);
    }
    if (min_y < 0){
        height += min_y;
        min_y = 0;
        height = std::max<ptrdiff_t>(height, 0);
    }

    return image.sub_image(min_x, min_y, width, height);
}


QImage extract_box_copy(const QImage& image, const ImagePixelBox& box){
    return image.copy(box.min_x, box.min_y, box.width(), box.height());
}
QImage extract_box_copy(const QImage& image, const ImageFloatBox& box){
    return image.copy(
        (pxint_t)(image.width() * box.x + 0.5),
        (pxint_t)(image.height() * box.y + 0.5),
        (pxint_t)(image.width() * box.width + 0.5),
        (pxint_t)(image.height() * box.height + 0.5)
    );
}
QImage extract_box(const QImage& image, const ImageFloatBox& box, int offset_x, int offset_y){
    return image.copy(
        (pxint_t)(image.width() * box.x + 0.5) + offset_x,
        (pxint_t)(image.height() * box.y + 0.5) + offset_y,
        (pxint_t)(image.width() * box.width + 0.5),
        (pxint_t)(image.height() * box.height + 0.5)
    );
}


ImageFloatBox translate_to_parent(
    const ConstImageRef& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
){
    double width = original_image.width();
    double height = original_image.height();
    pxint_t box_x = (pxint_t)(width * inference_box.x + 0.5);
    pxint_t box_y = (pxint_t)(height * inference_box.y + 0.5);
    return ImageFloatBox(
        (box_x + box.min_x) / width,
        (box_y + box.min_y) / height,
        (box.max_x - box.min_x) / width,
        (box.max_y - box.min_y) / height
    );
}


ImagePixelBox floatbox_to_pixelbox(size_t width, size_t height, const ImageFloatBox& float_box){
    return ImagePixelBox(
        (pxint_t)(width * float_box.x + 0.5),
        (pxint_t)(height * float_box.y + 0.5),
        (pxint_t)(width * (float_box.x + float_box.width) + 0.5),
        (pxint_t)(height * (float_box.y + float_box.height) + 0.5)
    );
}
ImageFloatBox pixelbox_to_floatbox(size_t width, size_t height, const ImagePixelBox& pixel_box){
    double image_inverse_width = 1. / (double)width;
    double image_inverse_height = 1. / (double)height;
    return ImageFloatBox(
        pixel_box.min_x * image_inverse_width,
        pixel_box.min_y * image_inverse_height,
        pixel_box.width() * image_inverse_width,
        pixel_box.height() * image_inverse_height
    );
}
ImageFloatBox pixelbox_to_floatbox(const QImage& image, const ImagePixelBox& pixel_box){
    return pixelbox_to_floatbox(image.width(), image.height(), pixel_box);
}


ImagePixelBox extract_object_from_inner_feature(
    const ImagePixelBox& inner_relative_to_image,
    const ImageFloatBox& inner_relative_to_object
){
    double scale_x = inner_relative_to_image.width() / inner_relative_to_object.width;
    double scale_y = inner_relative_to_image.height() / inner_relative_to_object.height;

    double shift_x = inner_relative_to_image.min_x - inner_relative_to_object.x * scale_x;
    double shift_y = inner_relative_to_image.min_y - inner_relative_to_object.y * scale_y;

    return ImagePixelBox(
        (pxint_t)(shift_x + 0.5),
        (pxint_t)(shift_y + 0.5),
        (pxint_t)(shift_x + scale_x + 0.5),
        (pxint_t)(shift_y + scale_y + 0.5)
    );
}
QImage extract_object_from_inner_feature(
    const QImage& image,
    const ImagePixelBox& inner_relative_to_image,
    const ImageFloatBox& inner_relative_to_object
){
    return extract_box_copy(
        image,
        extract_object_from_inner_feature(
            inner_relative_to_image,
            inner_relative_to_object
        )
    );
}




}
