/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_Default.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

namespace PokemonAutomation{
namespace Kernels{




class PixelTest_Rgb32Euclidean_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Euclidean_Default(
        uint32_t expected, double max_euclidean_distance
    )
        : m_expected_r((expected & 0x00ff0000) >> 16)
        , m_expected_g((expected & 0x0000ff00) >> 8)
        , m_expected_b(expected & 0x000000ff)
        , m_max_distance_squared((uint32_t)(max_euclidean_distance * max_euclidean_distance))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE bool test_word(uint32_t pixel) const{
        uint32_t sum_sqr = 0;
        {
            uint32_t p = (pixel & 0x00ff0000) >> 16;
            p -= m_expected_r;
            sum_sqr += p * p;
        }
        {
            uint32_t p = (pixel & 0x0000ff00) >> 8;
            p -= m_expected_g;
            sum_sqr += p * p;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            p -= m_expected_b;
            sum_sqr += p * p;
        }
        return sum_sqr <= m_max_distance_squared;
    }

private:
    uint32_t m_expected_r;
    uint32_t m_expected_g;
    uint32_t m_expected_b;
    uint32_t m_max_distance_squared;
};



size_t filter_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_Default tester(
        expected, max_euclidean_distance
    );
    FilterImage_Rgb32_Default<PixelTest_Rgb32Euclidean_Default> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_Default tester(
        expected, max_euclidean_distance
    );
    ToBlackWhite_Rgb32_Default<PixelTest_Rgb32Euclidean_Default> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}







}
}
