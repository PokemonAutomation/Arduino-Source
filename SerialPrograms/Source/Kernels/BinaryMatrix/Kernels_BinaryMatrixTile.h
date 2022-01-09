/*  Binary Matrix Tile
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_H

#include <stdint.h>
#include <iostream>
#include "Common/Compiler.h"
#include "Kernels/Kernels_Arch.h"

#if 0
#elif defined PA_Arch_x64_AVX512
#include "Kernels_BinaryMatrixTile_x64_AVX512.h"
namespace PokemonAutomation{
namespace Kernels{
    using BinaryMatrixTile = BinaryTile_AVX512;
}
}
#elif defined PA_Arch_x64_AVX2
#include "Kernels_BinaryMatrixTile_x64_AVX2.h"
namespace PokemonAutomation{
namespace Kernels{
    using BinaryMatrixTile = BinaryTile_AVX2;
}
}
#elif defined PA_Arch_x64_SSE42
#include "Kernels_BinaryMatrixTile_x64_SSE42.h"
namespace PokemonAutomation{
namespace Kernels{
    using BinaryMatrixTile = BinaryTile_SSE42;
}
}
#else
#include "Kernels_BinaryMatrixTile_Default.h"
namespace PokemonAutomation{
namespace Kernels{
    using BinaryMatrixTile = BinaryTile_Default;
}
}
#endif


namespace PokemonAutomation{
namespace Kernels{


class TileIndex{
public:
//    TileIndex(pxint_t x, pxint_t y)
//        : m_index((uint32_t)x | ((uint64_t)y << 32))
//    {}
    TileIndex(size_t x, size_t y)
        : m_index((uint32_t)x | ((uint64_t)y << 32))
    {
        if ((x | y) & 0xffffffff00000000){
            std::cerr << "Pixel Overflow: (" << x << "," << y << ")" << std::endl;
        }
    }

    PA_FORCE_INLINE size_t x() const{ return (uint32_t)m_index; }
    PA_FORCE_INLINE size_t y() const{ return (uint32_t)(m_index >> 32); }

    PA_FORCE_INLINE friend bool operator<(const TileIndex& a, const TileIndex& b){
        return a.m_index < b.m_index;
    }

private:
    uint64_t m_index;
};




}
}
#endif
