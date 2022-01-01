/*  Waterfill Tile (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX2_H
#define PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX2_H

#include "Kernels/BinaryMatrixTile/Kernels_BinaryMatrixTile_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{


void waterfill_expand(const BinaryTile_AVX2& m, BinaryTile_AVX2& x);



}
}
#endif
