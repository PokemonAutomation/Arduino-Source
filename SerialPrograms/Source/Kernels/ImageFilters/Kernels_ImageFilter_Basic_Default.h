/*  Image Filters Basic (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_Default_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_Default_H

#include <stdint.h>
#include <cstddef>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_Default{
public:
    static const size_t VECTOR_SIZE = 1;

public:
    ImageFilter_RgbRange_Default(uint32_t replacement, uint32_t mins, uint32_t maxs, bool invert)
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



}
}
#endif
