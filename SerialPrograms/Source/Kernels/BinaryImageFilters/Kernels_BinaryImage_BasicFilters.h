/*  Binary Image Basic Filters
 *
 *  From: https://github.com/PokemonAutomation/
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


//  Selectively replace pixels in an image with the replacement color
//  according to the respective bits in the binary matrix.
//  If `replace_zero_bits` is true, replace pixels corresponding to the zero bits.
//  Else, replace those with the one bits.
void filter_by_mask(
    const PackedBinaryMatrix_IB& matrix,
    uint32_t* image, size_t bytes_per_row,
    uint32_t replacement_color,
    bool replace_zero_bits
);



//  Compress (image, bytes_per_row) into a binary_image represented as the binary matrix `matrix`
//  Pixels in the specified RGB ranges [`mins`, `maxs`] are assigned 1 in the binary matrix,
//  otherwise 0.
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t mins, uint32_t maxs
);

//  Helper struct to store filtering related data for the multi-filter overload of `compress_rgb32_to_binary_range()`.
//  It stores `matrix`, the filtering result: a binary matrix, where 1-bits are the pixels that are in the filter RGB
//  range [`mins`, `maxs`], and 0-bits are otherwise.
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

//  Compress (image, bytes_per_row) into multiple binary_images represented as binary matrices stored in
//  `CompressRgb32ToBinaryRangeFilter.matrix` for each `filters`. 
//  For each filter, pixels in the specified RGB ranges [`Filter.mins`, `Filter.maxs`] are assigned 1 in `Filter.matrix`,
//  otherwise 0.
//  This function is like multiple calls to the single-filter overload of `compress_rgb32_to_binary_range()`, but 
//  with the benefit of reducing passes over the entire image.
//  All matrices in `filters` must have the same dimensions.
void compress_rgb32_to_binary_range(
    const uint32_t* image, size_t bytes_per_row,
    CompressRgb32ToBinaryRangeFilter* filters, size_t filter_count
);




//  Compress (image, bytes_per_row) into a binary_image.
//  For each pixel, set to 1 if the Euclidean distance of the pixel color to the expected color <= max distance.
void compress_rgb32_to_binary_euclidean(
    const uint32_t* image, size_t bytes_per_row,
    PackedBinaryMatrix_IB& matrix,
    uint32_t expected_color, double max_euclidean_distance
);




}
}
#endif
