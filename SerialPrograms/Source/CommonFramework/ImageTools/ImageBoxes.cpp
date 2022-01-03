/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QImage>
#include "Common/Cpp/Exception.h"
#include "ImageBoxes.h"

namespace PokemonAutomation{


ImagePixelBox::ImagePixelBox(size_t p_min_x, size_t p_min_y, size_t p_max_x, size_t p_max_y)
    : min_x((pxint_t)p_min_x), min_y((pxint_t)p_min_y)
    , max_x((pxint_t)p_max_x), max_y((pxint_t)p_max_y)
{
    if (min_x != (int64_t)p_min_x || max_x != (int64_t)p_max_x){
        PA_THROW_StringException(
            "Pixel Overflow: x = (" + std::to_string(p_min_x) + "," + std::to_string(p_max_x) + ")"
        );
    }
    if (min_y != (int64_t)p_min_y || max_y != (int64_t)p_max_y){
        PA_THROW_StringException(
            "Pixel Overflow: y = (" + std::to_string(p_min_y) + "," + std::to_string(p_max_y) + ")"
        );
    }
}




QImage extract_box(const QImage& image, const ImagePixelBox& box){
    return image.copy(box.min_x, box.min_y, box.width(), box.height());
}
QImage extract_box(const QImage& image, const ImageFloatBox& box){
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
    const QImage& original_image,
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



}
