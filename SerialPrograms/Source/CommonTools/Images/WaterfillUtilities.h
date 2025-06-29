/*  Waterfill Utilities
 *
 *  From: https://github.com/PokemonAutomation/
 *  
 *  Util functions that process waterfill objects and matrices.
 */

#ifndef PokemonAutomation_CommonTools_WaterfillUtilities_H
#define PokemonAutomation_CommonTools_WaterfillUtilities_H

#include <functional>
#include <utility>
#include "CommonFramework/ImageTypes/BinaryImage.h"

namespace PokemonAutomation{
    class ImageRGB32;
    class ImageViewRGB32;
namespace Kernels{
namespace Waterfill{
    class WaterfillObject;
}
}

namespace ImageMatch{
    class WaterfillTemplateMatcher;
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
std::pair<PackedBinaryMatrix, size_t> remove_center_pixels(
    const Kernels::Waterfill::WaterfillObject& object,
    size_t num_removed_pixels_threshold
);

// Given an image first run waterfill (aka use a color filter) on it to detect pixels of a color range. Then for each connected
// componet of the detected pixel region (aka waterfill object), we check if the object is close to an image template by
// checking aspect ratio thresholds, area thresholds and RMSD threshold.
// If a template match is found, call the function `check_matched_object()` on it.
//
// image: the image.
// matcher: the template matcher holding the template. It is also responsible for checking aspect ratio thresholds.
// filters: each filter is parameterized by min and max color thresholds for detected pixels. For each filter, the function checks
//  the waterfill objects formed by the detected pixels.
// area_thresholds: min, max thresholds for number of pixels of a waterfill object.
// rmsd_threshold: RMSD threshold. If RMSD of the waterfill object and template is smaller than this threshold, consider it a match.
// check_matched_object: if a matcher is found, pass the matched object to this function. If the function returns true, stop the
//   entire template matching operation.
bool match_template_by_waterfill(
    const ImageViewRGB32 &image,
    const ImageMatch::WaterfillTemplateMatcher &matcher,
    const std::vector<std::pair<uint32_t, uint32_t>> &filters,
    const std::pair<size_t, size_t> &area_thresholds,
    double rmsd_threshold,
    std::function<bool(Kernels::Waterfill::WaterfillObject& object)> check_matched_object);

// Draw matrix on an image. Used for debugging the matrix.
// color: color of the pixels from the matrix to render on the image.
// offset_x, offset_y: the offset of the matrix when rendered on the image.
void draw_matrix_on_image(
    const PackedBinaryMatrix& matrix,
    uint32_t color, ImageRGB32& image, size_t offset_x, size_t offset_y
);

// Draw waterfill object on an image. Used for debugging the waterfill object.
// Note: the object must have WaterfillObject.object computed.
// color: color of the pixels from the object to render on the image.
// offset_x, offset_y: the offset of the object when rendered on the image.
void draw_object_on_image(
    const Kernels::Waterfill::WaterfillObject& object,
    const uint32_t& color, ImageRGB32& image, size_t offset_x, size_t offset_y
);

}
#endif
