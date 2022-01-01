/*  Waterfill Tile (SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_x64_SSE41_H
#define PokemonAutomation_Kernels_Waterfill_Tile_x64_SSE41_H

#include "Kernels/BinaryMatrixTile/Kernels_BinaryMatrixTile_x64_SSE41.h"

namespace PokemonAutomation{
namespace Kernels{


void waterfill_expand(const BinaryTile_SSE41& m, BinaryTile_SSE41& x);



}
}
#endif
