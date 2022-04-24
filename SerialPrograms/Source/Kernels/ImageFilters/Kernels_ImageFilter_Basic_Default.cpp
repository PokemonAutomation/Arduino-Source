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

public:
    ImageFilter_RgbRange_Default(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(replacement)
        , m_invert(invert ? 1 : 0)
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
        ret ^= m_invert;
        out[0] = ret ? pixel : m_replacement;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_replacement;
    const uint32_t m_invert;
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



void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, filter0, out0, bytes_per_row0);
    count0 = filter0.count();
}



void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_rbg32<ImageFilter_RgbRange_Default>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}







}
}
