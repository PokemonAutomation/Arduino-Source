/*  Waterfill Tile (AVX512)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX512_H
#define PokemonAutomation_Kernels_Waterfill_Tile_x64_AVX512_H

#include "Kernels/BinaryMatrix/Kernels_BinaryMatrixTile_x64_AVX512.h"

namespace PokemonAutomation{
namespace Kernels{



//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
bool find_bit(size_t& x, size_t& y, const BinaryTile_AVX512& tile);



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
__m512i popcount_indexsum(__m512i& sum_index, __m512i x);
uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_AVX512& tile
);



//  Run waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x".
void waterfill_expand(const BinaryTile_AVX512& m, BinaryTile_AVX512& x);



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
bool waterfill_touch_top   (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
bool waterfill_touch_bottom(const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
bool waterfill_touch_left  (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);
bool waterfill_touch_right (const BinaryTile_AVX512& mask, BinaryTile_AVX512& tile, const BinaryTile_AVX512& border);




}
}
#endif
