/*  Waterfill Tile (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX512_H

#include "Kernels/BinaryMatrixTile/Kernels_BinaryMatrixTile_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{


void waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x);



}
}
#endif
