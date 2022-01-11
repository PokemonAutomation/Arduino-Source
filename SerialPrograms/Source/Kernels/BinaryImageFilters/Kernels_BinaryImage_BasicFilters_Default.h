/*  Binary Image Basic Filters (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_Default_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_Default_H

#include <stdint.h>
#include <cstddef>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{



class Compressor_RgbRange_Default{
public:
    Compressor_RgbRange_Default(
        uint8_t min_alpha, uint8_t max_alpha,
        uint8_t min_red, uint8_t max_red,
        uint8_t min_green, uint8_t max_green,
        uint8_t min_blue, uint8_t max_blue
    )
        : m_minB(min_blue)
        , m_maxB(max_blue)
        , m_minG(min_green << 8)
        , m_maxG(max_green << 8)
        , m_minR(min_red << 16)
        , m_maxR(max_red << 16)
        , m_minA(min_alpha << 24)
        , m_maxA(max_alpha << 24)
    {}

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count = 64) const{
        uint64_t bits = 0;
        size_t c = 0;
        do{
            bits |= convert1(pixels[c]) << c;
            c++;
        }while (c < count);
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert1(uint32_t pixel) const{
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
        return ret;
    }

private:
    uint32_t m_minB;
    uint32_t m_maxB;
    uint32_t m_minG;
    uint32_t m_maxG;
    uint32_t m_minR;
    uint32_t m_maxR;
    uint32_t m_minA;
    uint32_t m_maxA;
};



class Filter_Default{
public:
    Filter_Default(uint32_t replacement, bool replace_if_zero)
        : m_replacement(replacement)
        , m_replace_if_zero(replace_if_zero ? 1 : 0)
    {}

    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t start = 0, size_t stop = 64) const{
        size_t c = start;
        do{
            filter1(bits & 1, pixels[c]);
            bits >>= 1;
            c++;
        }while (c < stop);
    }

private:
    PA_FORCE_INLINE uint32_t filter1(uint64_t bit, uint32_t pixel) const{
        return (bit ^ m_replace_if_zero) ? m_replacement : pixel;
    }

private:
    uint32_t m_replacement;
    uint64_t m_replace_if_zero;
};




}
}
#endif
