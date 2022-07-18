/*  Waterfill Utilities
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *  
 *  Util functions that process waterfill objects and matrices.
 */

#ifndef PokemonAutomation_CommonFramework_WaterfillUtilities_H
#define PokemonAutomation_CommonFramework_WaterfillUtilities_H

#include "CommonFramework/ImageTypes/BinaryImage.h"

#include <utility>

class QImage;

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}

// Remove pixels close to center of mass of the water fill object.
// Pixels are removed based on the distance to the center of mass. The code will choose a distance threshold as large as possible
// while the number of the removed pixels is smaller than `num_removed_pixels_threshold`.
// Return a binary matrix representing the remaining pixels and the distance threshold which is defined as the minimum distance of
// the remaining pixels towards the original center of mass.
// Note: `object.object` must be constructed before calling this function.
// Example usage for analyzing the shape of the object:
// auto std::tie(matrix, distance) = remove_center_pixels(object, (size_t)(0.85 * object.area));
// auto session = make_WaterfillSession(matrix);
// ...
std::pair<PackedBinaryMatrix2, size_t> remove_center_pixels(
    const Kernels::Waterfill::WaterfillObject& object,
    size_t num_removed_pixels_threshold
);

// Draw matrix on an image. Used for debugging the matrix.
// color: color of the pixels from the matrix to render on the image.
// offset_x, offset_y: the offset of the matrix when rendered on the image.
void draw_matrix_on_image(
    const PackedBinaryMatrix2& matrix,
    uint32_t color, QImage& image, size_t offset_x, size_t offset_y
);

// Draw waterfill object on an image. Used for debugging the waterfill object.
// Note: the object must have WaterfillObject.object computed.
// color: color of the pixels from the object to render on the image.
// offset_x, offset_y: the offset of the object when rendered on the image.
void draw_object_on_image(
    const Kernels::Waterfill::WaterfillObject& object,
    const uint32_t& color, QImage& image, size_t offset_x, size_t offset_y
);

}
#endif
