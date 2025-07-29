/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Color.h"
#include "Common/Cpp/Rectangle.tpp"
#include "Kernels/Waterfill/Kernels_Waterfill_Types.h"
#include "CommonFramework/ImageTypes/ImageViewHSV32.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "ImageBoxes.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


template struct Rectangle<size_t>;


ImagePixelBox::ImagePixelBox(size_t p_min_x, size_t p_min_y, size_t p_max_x, size_t p_max_y)
    : Rectangle(p_min_x, p_min_y, p_max_x, p_max_y)
{
    const size_t MAX_VALUE = 0xffffffff;

    //  Check for potential negative values being passed here.
    if (min_x > MAX_VALUE){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Pixel Overflow: min_x = " + std::to_string(min_x)
        );
    }
    if (min_y > MAX_VALUE){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Pixel Overflow: min_y = " + std::to_string(min_y)
        );
    }
    if (max_x > MAX_VALUE){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Pixel Overflow: max_x = " + std::to_string(max_x)
        );
    }
    if (max_y > MAX_VALUE){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION, "Pixel Overflow: max_y = " + std::to_string(max_y)
        );
    }
}


ImagePixelBox::ImagePixelBox(const Kernels::Waterfill::WaterfillObject& object)
    : ImagePixelBox(object.min_x, object.min_y, object.max_x, object.max_y)
{}


void ImagePixelBox::clip(size_t image_width, size_t image_height){
    min_x = std::max((size_t)0, min_x);
    min_y = std::max((size_t)0, min_y);
    max_x = std::min(max_x, image_width);
    max_y = std::min(max_y, image_height);
}
void ImagePixelBox::clip(const ImagePixelBox& box){
    min_x = std::max(min_x, box.min_x);
    min_y = std::max(min_y, box.min_y);
    max_x = std::min(max_x, box.max_x);
    max_y = std::min(max_y, box.max_y);
}

ImagePixelBox ImagePixelBox::expand_as(size_t per_side_increase) const{
    size_t new_min_x = min_x >= per_side_increase ? min_x - per_side_increase : 0;
    size_t new_min_y = min_y >= per_side_increase ? min_y - per_side_increase : 0;
    return ImagePixelBox(new_min_x, new_min_y, max_x + per_side_increase, max_y + per_side_increase);
}

size_t ImagePixelBox::distance_x(const ImagePixelBox& box) const{
    size_t min_x = std::max(this->min_x, box.min_x);
    size_t max_x = std::min(this->max_x, box.max_x);
    if (min_x >= max_x){
        return min_x - max_x;
    }
    return 0;
}

size_t ImagePixelBox::distance_y(const ImagePixelBox& box) const{
    size_t min_y = std::max(this->min_y, box.min_y);
    size_t max_y = std::min(this->max_y, box.max_y);
    if (min_y >= max_y){
        return min_y - max_y;
    }
    return 0;
}

size_t ImagePixelBox::distance_to_point_x(const size_t x) const{
    size_t min_x = std::max(this->min_x, x);
    size_t max_x = std::min(this->max_x, x);
    return min_x - max_x;
}
//  The distance to a point on y axis. If the point is in the box, the distance is 0.
size_t ImagePixelBox::distance_to_point_y(const size_t y) const{
    size_t min_y = std::max(this->min_y, y);
    size_t max_y = std::min(this->max_y, y);
    return min_y - max_y;
}
//  The distance from the box center to a point on x axis
size_t ImagePixelBox::center_distance_to_point_x(const size_t x) const{
    const size_t cx = center_x();
    if (cx >= x){
        return cx - x;
    }
    return x - cx;
}
//  The distance from the box center to a point on y axis
size_t ImagePixelBox::center_distance_to_point_y(const size_t y) const{
    const size_t cy = center_y();
    if (cy >= y){
        return cy - y;
    }
    return y - cy;
}


ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImagePixelBox& box){
    return image.sub_image(box.min_x, box.min_y, box.width(), box.height());
}
ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImageFloatBox& box){
    size_t min_x = (size_t)(image.width() * box.x + 0.5);
    size_t min_y = (size_t)(image.height() * box.y + 0.5);
    size_t width = (size_t)(image.width() * box.width + 0.5);
    size_t height = (size_t)(image.height() * box.height + 0.5);
    return image.sub_image(min_x, min_y, width, height);
}
ImageViewRGB32 extract_box_reference(const ImageViewRGB32& image, const ImageFloatBox& box, ptrdiff_t offset_x, ptrdiff_t offset_y){
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


ImageViewHSV32 extract_box_reference(const ImageViewHSV32& image, const ImagePixelBox& box){
    return image.sub_image(box.min_x, box.min_y, box.width(), box.height());
}
ImageViewHSV32 extract_box_reference(const ImageViewHSV32& image, const ImageFloatBox& box){
    size_t min_x = (size_t)(image.width() * box.x + 0.5);
    size_t min_y = (size_t)(image.height() * box.y + 0.5);
    size_t width = (size_t)(image.width() * box.width + 0.5);
    size_t height = (size_t)(image.height() * box.height + 0.5);
    return image.sub_image(min_x, min_y, width, height);
}
ImageViewHSV32 extract_box_reference(const ImageViewHSV32& image, const ImageFloatBox& box, ptrdiff_t offset_x, ptrdiff_t offset_y){
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



ImageFloatBox translate_to_parent(
    const ImageViewRGB32& original_image,
    const ImageFloatBox& inference_box,
    const ImagePixelBox& box
){
    double width = (double)original_image.width();
    double height = (double)original_image.height();
    ptrdiff_t box_x = (ptrdiff_t)(width * inference_box.x + 0.5);
    ptrdiff_t box_y = (ptrdiff_t)(height * inference_box.y + 0.5);
    return ImageFloatBox(
        (box_x + box.min_x) / width,
        (box_y + box.min_y) / height,
        (box.max_x - box.min_x) / width,
        (box.max_y - box.min_y) / height
    );
}


ImagePixelBox floatbox_to_pixelbox(size_t width, size_t height, const ImageFloatBox& float_box){
    return ImagePixelBox(
        (size_t)std::max<double>(width * float_box.x + 0.5, 0),
        (size_t)std::max<double>(height * float_box.y + 0.5, 0),
        (size_t)(width * (float_box.x + float_box.width) + 0.5),
        (size_t)(height * (float_box.y + float_box.height) + 0.5)
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
ImageFloatBox pixelbox_to_floatbox(const ImageViewRGB32& image, const ImagePixelBox& pixel_box){
    return pixelbox_to_floatbox(image.width(), image.height(), pixel_box);
}


ImagePixelBox extract_object_from_inner_feature(
    size_t width, size_t height,
    const ImagePixelBox& inner_relative_to_image,
    const ImageFloatBox& inner_relative_to_object
){
    double scale_x = inner_relative_to_image.width() / inner_relative_to_object.width;
    double scale_y = inner_relative_to_image.height() / inner_relative_to_object.height;

    double shift_x = inner_relative_to_image.min_x - inner_relative_to_object.x * scale_x;
    double shift_y = inner_relative_to_image.min_y - inner_relative_to_object.y * scale_y;
    shift_x = std::max<double>(shift_x, 0);
    shift_y = std::max<double>(shift_y, 0);

    size_t max_x = (size_t)(shift_x + scale_x + 0.5);
    size_t max_y = (size_t)(shift_y + scale_y + 0.5);
    max_x = std::min(max_x, width);
    max_y = std::min(max_y, height);

    return ImagePixelBox(
        (size_t)(shift_x + 0.5),
        (size_t)(shift_y + 0.5),
        max_x,
        max_y
    );
}


void draw_box(ImageRGB32& image, const ImagePixelBox& pixel_box, uint32_t color, size_t thickness){
    if (thickness == 0 || image.width() == 0 || image.height() == 0){
        return;
    }

    auto clamp_x = [&](ptrdiff_t x){
        x = std::max<ptrdiff_t>(x, 0);
        return std::min(x, (ptrdiff_t)image.width() - 1);
    };
    auto clamp_y = [&](ptrdiff_t y){
        y = std::max<ptrdiff_t>(y, 0);
        return std::min(y, (ptrdiff_t)image.height() - 1);
    };

    auto draw_solid_rect = [&](ptrdiff_t start_x, ptrdiff_t start_y, ptrdiff_t end_x, ptrdiff_t end_y){
        start_x = clamp_x(start_x);
        end_x = clamp_x(end_x);
        start_y = clamp_y(start_y);
        end_y = clamp_y(end_y);
        for (ptrdiff_t y = start_y; y <= end_y; ++y){
            for (ptrdiff_t x = start_x; x <= end_x; ++x){
                // setPixelColor(x, y, qColor);
                image.pixel(x, y) = color;
            }
        }
    };

    ptrdiff_t lo = ((ptrdiff_t)thickness - 1) / 2; // lower offset
    ptrdiff_t uo = (ptrdiff_t)thickness - lo - 1; // upper offset

    // draw the upper horizontal line
    draw_solid_rect(pixel_box.min_x-lo, pixel_box.min_y-lo, pixel_box.max_x+uo-1, pixel_box.min_y+uo);
    // draw the lower horizontal line
    draw_solid_rect(pixel_box.min_x-lo, pixel_box.max_y-lo-1, pixel_box.max_x+uo-1, pixel_box.max_y+uo-1);
    // draw the left vertical line
    draw_solid_rect(pixel_box.min_x-lo, pixel_box.min_y-lo, pixel_box.min_x+uo, pixel_box.max_y+uo-1);
    // draw the right vertical line
    draw_solid_rect(pixel_box.max_x-lo-1, pixel_box.min_y-lo, pixel_box.max_x+uo-1, pixel_box.max_y+uo-1);
}



}
