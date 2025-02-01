/*  Image Filters Basic (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <stdint.h>
#include <cstddef>
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilter_RgbRange_Default(uint32_t mins, uint32_t maxs, uint32_t replacement_color,
        bool replace_color_within_range)
        : m_replacement_color(replacement_color)
        , m_replace_color_within_range(replace_color_within_range ? 1 : 0)
        , m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint64_t ret = 1;
        {
            uint32_t p = pixel & 0xff000000;
            ret &= p >= m_minA;
            ret &= p <= m_maxA;
        }
        {
            uint32_t p = pixel & 0x00ff0000;
            ret &= p >= m_minR;
            ret &= p <= m_maxR;
        }
        {
            uint32_t p = pixel & 0x0000ff00;
            ret &= p >= m_minG;
            ret &= p <= m_maxG;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            ret &= p >= m_minB;
            ret &= p <= m_maxB;
        }
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
    const uint32_t m_minB;
    const uint32_t m_maxB;
    const uint32_t m_minG;
    const uint32_t m_maxG;
    const uint32_t m_minR;
    const uint32_t m_maxR;
    const uint32_t m_minA;
    const uint32_t m_maxA;
    size_t m_count;
};
size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement_color, bool replace_color_within_range
){
    ImageFilter_RgbRange_Default filter(mins, maxs, replacement_color, replace_color_within_range);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilter_RgbRange_Default>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





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








class ToBlackWhite_RgbRange_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ToBlackWhite_RgbRange_Default(uint32_t mins, uint32_t maxs, bool in_range_black)
        : m_in_range_black(in_range_black ? 1 : 0)
        , m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint64_t ret = 1;
        {
            uint32_t p = pixel & 0xff000000;
            ret &= p >= m_minA;
            ret &= p <= m_maxA;
        }
        {
            uint32_t p = pixel & 0x00ff0000;
            ret &= p >= m_minR;
            ret &= p <= m_maxR;
        }
        {
            uint32_t p = pixel & 0x0000ff00;
            ret &= p >= m_minG;
            ret &= p <= m_maxG;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            ret &= p >= m_minB;
            ret &= p <= m_maxB;
        }
        m_count += ret;
        ret ^= m_in_range_black;
        out[0] = ret ? 0xffffffff : 0xff000000;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_in_range_black;
    const uint32_t m_minB;
    const uint32_t m_maxB;
    const uint32_t m_minG;
    const uint32_t m_maxG;
    const uint32_t m_minR;
    const uint32_t m_maxR;
    const uint32_t m_minA;
    const uint32_t m_maxA;
    size_t m_count;
};



size_t to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ToBlackWhite_RgbRange_Default filter(mins, maxs, in_range_black);
    filter_per_pixel(image, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    to_blackwhite_rbg32<ToBlackWhite_RgbRange_Default>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}


























}
}
