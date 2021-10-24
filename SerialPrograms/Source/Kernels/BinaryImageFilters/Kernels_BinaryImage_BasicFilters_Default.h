/*  Binary Image Basic Filters (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_Default_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_Default_H

#include "Kernels/BinaryImage/Kernels_BinaryImage_Default.h"

namespace PokemonAutomation{
namespace Kernels{


class MinRgbFilter_Default{
public:
    MinRgbFilter_Default(
        uint8_t min_alpha, uint8_t min_red, uint8_t min_green, uint8_t min_blue
    )
        : m_minB(min_blue)
        , m_minG(min_green << 8)
        , m_minR(min_red << 16)
        , m_minA(min_alpha << 24)
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
        uint64_t ret = (pixel & 0xff000000) >= m_minA;
        ret &= (pixel & 0x00ff0000) >= m_minR;
        ret &= (pixel & 0x0000ff00) >= m_minG;
        ret &= (pixel & 0x000000ff) >= m_minB;
        return ret;
    }

private:
    uint32_t m_minB;
    uint32_t m_minG;
    uint32_t m_minR;
    uint32_t m_minA;
};




}
}
#endif
