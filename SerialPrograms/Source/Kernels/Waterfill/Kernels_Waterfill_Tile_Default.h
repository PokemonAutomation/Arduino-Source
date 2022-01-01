/*  Waterfill Tile (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_Default_H
#define PokemonAutomation_Kernels_Waterfill_Tile_Default_H

#include "Kernels/BinaryMatrixTile/Kernels_BinaryMatrixTile_Default.h"

namespace PokemonAutomation{
namespace Kernels{


void waterfill_expand(const BinaryTile_Default& m, BinaryTile_Default& x);



}
}
#endif
