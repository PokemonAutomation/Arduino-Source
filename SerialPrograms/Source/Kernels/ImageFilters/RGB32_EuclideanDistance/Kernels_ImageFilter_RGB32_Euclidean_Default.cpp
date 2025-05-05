/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

namespace PokemonAutomation{
namespace Kernels{



class ImageFilter_RgbEuclidean_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilter_RgbEuclidean_Default(uint32_t expected_color, double max_euclidean_distance, uint32_t replacement_color, bool replace_color_within_range)
        : m_replacement_color(replacement_color)
        , m_replace_color_within_range(replace_color_within_range ? 1 : 0)
        , m_expected_r((expected_color & 0x00ff0000) >> 16)
        , m_expected_g((expected_color & 0x0000ff00) >> 8)
        , m_expected_b(expected_color & 0x000000ff)
        , m_max_distance_squared((uint32_t)(max_euclidean_distance * max_euclidean_distance))
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint32_t ret = 1;
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
        ret = sum_sqr <= m_max_distance_squared;
        m_count += ret;
        ret ^= m_replace_color_within_range;
        out[0] = ret ? pixel : m_replacement_color;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_replacement_color;
    const uint32_t m_replace_color_within_range;
    uint32_t m_expected_r;
    uint32_t m_expected_g;
    uint32_t m_expected_b;
    uint32_t m_max_distance_squared;
    size_t m_count;
};
size_t filter_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected_color, double max_euclidean_distance,
    uint32_t replacement_color, bool replace_color_within_range
){
    ImageFilter_RgbEuclidean_Default filter(expected_color, max_euclidean_distance, replacement_color, replace_color_within_range);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
