/*  Image Boxes
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "ImageBoxes.h"

namespace PokemonAutomation{


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


}
