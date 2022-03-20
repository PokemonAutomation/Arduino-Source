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
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
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
        ret ^= m_invert;
        out[0] = ret ? pixel : m_replacement;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        process_full(out, in);
    }


private:
    uint32_t m_replacement;
    uint32_t m_invert;
    uint32_t m_minB;
    uint32_t m_maxB;
    uint32_t m_minG;
    uint32_t m_maxG;
    uint32_t m_minR;
    uint32_t m_maxR;
    uint32_t m_minA;
    uint32_t m_maxA;
};



void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, out0, bytes_per_row0, filter0);
}
void filter2_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_Default filter1(mins1, maxs1, replacement1, invert1);
    filter2_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1
    );
}
void filter4_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_Default filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_Default filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_Default filter3(mins3, maxs3, replacement3, invert3);
    filter4_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1,
        out2, bytes_per_row2, filter3,
        out3, bytes_per_row3, filter1
    );
}




}
}
