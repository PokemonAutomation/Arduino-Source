/*  Waterfill Tile (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_Default_H
#define PokemonAutomation_Kernels_Waterfill_Tile_Default_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_Default.h"

namespace PokemonAutomation{
namespace Kernels{



//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
bool find_bit(size_t& x, size_t& y, const BinaryTile_Default& tile);



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
uint64_t popcount_indexsum(uint64_t& sum_index, uint64_t x);
uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_Default& tile
);



//  Run waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
void waterfill_expand(const BinaryTile_Default& m, BinaryTile_Default& x);



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
bool waterfill_touch_top   (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
bool waterfill_touch_bottom(const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
bool waterfill_touch_left  (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
bool waterfill_touch_right (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);



}
}
#endif
