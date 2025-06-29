/*  Image Filters Basic (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <cstddef>
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_Green_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilter_Green_Default(uint32_t replacement_color, uint8_t rgb_gap)
        : m_replacement_color(replacement_color)
        , m_rgb_gap(rgb_gap)
        , m_count(0)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint64_t ret = 1;

        uint16_t red = ((pixel >> 16) & 0xff);
        uint16_t green = ((pixel >> 8) & 0xff);
        uint16_t blue = ((pixel) & 0xff);

        ret &= (green >= red + m_rgb_gap) && (green >= blue + m_rgb_gap);


        m_count += ret;
        out[0] = ret ? pixel : m_replacement_color;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_replacement_color;
    const uint16_t m_rgb_gap;
    size_t m_count;
};


size_t filter_green_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement_color, uint8_t rgb_gap
){
    ImageFilter_Green_Default filter(replacement_color, rgb_gap);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}

























}
}
