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

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class Compressor_RgbRange_Default{
public:
    Compressor_RgbRange_Default(uint32_t mins, uint32_t maxs)
        : m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
    {}

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count = 64) const{
        uint64_t bits = 0;
        size_t c = 0;
        while (c < count){
            bits |= convert1(pixels[c]) << c;
            c++;
        }
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



class BinaryFilter_RgbRange_Default{
public:
    BinaryFilter_RgbRange_Default(uint32_t replacement, bool replace_if_zero)
        : m_replacement(replacement)
        , m_replace_if_zero(replace_if_zero ? 1 : 0)
    {}

    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count = 64) const{
        size_t c = 0;
        while (c < count){
            pixels[c] = filter1(bits & 1, pixels[c]);
            bits >>= 1;
            c++;
        }
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
