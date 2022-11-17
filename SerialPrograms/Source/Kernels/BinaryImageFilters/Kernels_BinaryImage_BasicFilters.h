/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 *      Perform a filter over an image and store the results in a binary image.
 *
 *  The parameter "binary_image" specifies the dimensions of the image.
 *  The parameter itself is overwritten by result.
 *
 *  Example Usage:
 *
 *      BinaryImage binary_image(1280, 720);
 *
 *      filter_min_rgb32(
 *          binary_image,
 *          image, bytes_per_row,
 *          0xff, 0x80, 0x80, 0x80
 *      );
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix.h"

namespace PokemonAutomation{
namespace Kernels{



//  Selectively replace each pixel in an image with the specified pixel
//  according to the respective bit in the binary matrix.
void filter_by_mask(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replace_with,
    bool replace_if_zero    //  If false, replace if one.
);



//  Compress (image, bytes_per_row) into a binary_image.
//  Use the specified RGB ranges to determine whether each pixel
//  becomes a 0 or a 1.
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);


//  Same as above, but multiple filters.
//  The purpose is to reduce passes over the entire image.
//  All matricies must have the same dimensions.
struct CompressRgb32ToBinaryRangeFilter{
    PackedBinaryMatrix_IB& matrix;
    uint32_t mins;
    uint32_t maxs;

    CompressRgb32ToBinaryRangeFilter(PackedBinaryMatrix_IB& p_matrix, uint32_t p_mins, uint32_t p_maxs)
        : matrix(p_matrix)
        , mins(p_mins)
        , maxs(p_maxs)
    {}
};
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);




//  Compress (image, bytes_per_row) into a binary_image.
//  For each pixel, set to 1 if distance is within the expected value.
void compress_rgb32_to_binary_euclidean(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected, double max_euclidean_distance
);




}
}
#endif
