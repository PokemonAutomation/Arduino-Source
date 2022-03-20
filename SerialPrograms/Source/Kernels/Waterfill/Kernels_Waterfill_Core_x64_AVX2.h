/*  Waterfill Core (AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_x64_AVX2_H
#define PokemonAutomation_Kernels_Waterfill_Core_x64_AVX2_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{
struct Waterfill_x64_AVX2{



//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static bool find_bit(size_t& x, size_t& y, const BinaryTile_AVX2& tile);



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static void boundaries(
    const BinaryTile_AVX2& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
);



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static __m256i popcount_indexsum(__m256i& sum_index, __m256i x);
static uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_AVX2& tile
);



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
static void Waterfill_expand(const BinaryTile_AVX2& m, BinaryTile_AVX2& x);



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static bool Waterfill_touch_top   (const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border);
static bool Waterfill_touch_bottom(const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border);
static bool Waterfill_touch_left  (const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border);
static bool Waterfill_touch_right (const BinaryTile_AVX2& mask, BinaryTile_AVX2& tile, const BinaryTile_AVX2& border);



};
}
}
}
#endif
