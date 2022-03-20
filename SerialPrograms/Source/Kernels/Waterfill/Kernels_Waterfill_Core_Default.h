/*  Waterfill Core (Default)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_Default_H
#define PokemonAutomation_Kernels_Waterfill_Core_Default_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
struct Waterfill_Default{



//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static bool find_bit(size_t& x, size_t& y, const BinaryTile_Default& tile);



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static void boundaries(
    const BinaryTile_Default& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
);



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static uint64_t popcount_indexsum(uint64_t& sum_index, uint64_t x);
static uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_Default& tile
);



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
static void Waterfill_expand(const BinaryTile_Default& m, BinaryTile_Default& x);



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static bool Waterfill_touch_top   (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
static bool Waterfill_touch_bottom(const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
static bool Waterfill_touch_left  (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);
static bool Waterfill_touch_right (const BinaryTile_Default& mask, BinaryTile_Default& tile, const BinaryTile_Default& border);



};
}
}
}
#endif
