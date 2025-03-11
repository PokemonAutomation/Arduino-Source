/*  Waterfill Core (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64xH_Default_H
#define PokemonAutomation_Kernels_Waterfill_Core_64xH_Default_H

#include "Kernels/Kernels_BitScan.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


PA_FORCE_INLINE uint64_t bitreverse64(uint64_t x){
    uint64_t r0, r1;

#if 0
#elif __GNUC__
    r0 = __builtin_bswap64(x);
#elif __INTEL_COMPILER
    r0 = _bswap64(x);
#elif _MSC_VER
    r0 = _byteswap_uint64(x);
#else
#error "No byte-swap for this compiler."
#endif

    r1 = r0 >> 4;
    r0 = r0 << 4;
    r1 &= 0x0f0f0f0f0f0f0f0full;
    r0 &= 0xf0f0f0f0f0f0f0f0ull;
    r0 |= r1;

    r1 = r0 >> 2;
    r0 = r0 << 2;
    r1 &= 0x3333333333333333ull;
    r0 &= 0xccccccccccccccccull;
    r0 |= r1;

    r1 = r0 >> 1;
    r0 = r0 << 1;
    r1 &= 0x5555555555555555ull;
    r0 &= 0xaaaaaaaaaaaaaaaaull;
    r0 |= r1;

    return r0;
}




template <typename Tile>
struct Waterfill_64xH_Default_ProcessedMask{
    static_assert(Tile::WIDTH == 64);

    uint64_t m[Tile::HEIGHT];   //  Copy of the masks.
    uint64_t b[Tile::HEIGHT];   //  Bit-reversed copy of the masks.

    PA_FORCE_INLINE Waterfill_64xH_Default_ProcessedMask(
        const Tile& mask,
        uint64_t x[Tile::HEIGHT]
    ){
        for (size_t c = 0; c < Tile::HEIGHT; c++){
            m[c] = x[c] | mask.row(c);
            b[c] = bitreverse64(m[c]);
        }
    }
};

template <typename Tile>
PA_FORCE_INLINE bool keep_going(
    const Waterfill_64xH_Default_ProcessedMask<Tile>& mask,
    uint64_t m[Tile::HEIGHT], uint64_t x[Tile::HEIGHT]
){
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        m[c] = ~x[c] & mask.m[c];
    }

    uint64_t changed = x[0] & ((m[0] << 1) | m[1]);

    size_t c = 1;
    for (; c < Tile::HEIGHT - 1; c++){
        changed |= x[c] & ((m[c] << 1) | m[c - 1] | m[c + 1]);
    }

    changed |= x[c] & ((m[c] << 1) | m[c - 1]);

    return changed;
}

template <typename Tile>
PA_FORCE_INLINE void expand_forward(
    const Waterfill_64xH_Default_ProcessedMask<Tile>& mask,
    uint64_t x[Tile::HEIGHT]
){
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        uint64_t m = mask.m[c];
        uint64_t s = x[c] + m;
        s ^= m;
        s &= m;
        x[c] |= s;
    }
}

template <typename Tile>
PA_FORCE_INLINE void expand_reverse(
    const Waterfill_64xH_Default_ProcessedMask<Tile>& mask,
    uint64_t x[Tile::HEIGHT]
){
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        uint64_t m = mask.m[c];
        uint64_t s = bitreverse64(bitreverse64(x[c]) + mask.b[c]);
        s ^= m;
        s &= m;
        x[c] |= s;
    }
}



template <typename Tile>
PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64xH_Default_ProcessedMask<Tile>& mask,
    uint64_t x[Tile::HEIGHT]
){
    size_t dn = 0;
    size_t up = Tile::HEIGHT - 1;
    for (; dn < Tile::HEIGHT - 1;){
        x[dn + 1] |= x[dn] & mask.m[dn + 1];
        x[up - 1] |= x[up] & mask.m[up - 1];
        dn++;
        up--;
    }
}





template <typename Tile>
struct Waterfill_64xH_Default{


static PA_FORCE_INLINE uint64_t row_or(const Tile& tile){
    static_assert(Tile::WIDTH == 64);

    uint64_t v0 = 0;
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        v0 |= tile.row(c);
    }

    return v0;
}

//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static PA_FORCE_INLINE bool find_bit(size_t& x, size_t& y, const Tile& tile){
    uint64_t anything = row_or(tile);
    if (!anything){
        return false;
    }
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        size_t pos;
        if (trailing_zeros(pos, tile.row(c))){
            x = pos;
            y = c;
            return true;
        }
    }
    return false;
}


//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static PA_FORCE_INLINE void boundaries(
    const Tile& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    uint64_t all_or = row_or(tile);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    min_y = 0;
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        if (tile.row(c) != 0){
            min_y = c;
            break;
        }
    }
    max_y = 0;
    for (size_t c = Tile::HEIGHT; c > 0; c--){
        if (tile.row(c - 1) != 0){
            max_y = c;
            break;
        }
    }
}



//  Area + Center of Gravity:
//      Compute the sum of the index of each set bit.
//      Returns the popcount.
static PA_FORCE_INLINE uint64_t popcount_indexsum(uint64_t& sum_index, uint64_t x){
    //  1 -> 2
    uint64_t sum_high;
    uint64_t pop_high = (x >> 1) & 0x5555555555555555;
    uint64_t sumxaxis = pop_high;
    uint64_t popcount = (x & 0x5555555555555555) + pop_high;

    //  2 -> 4
    sum_high = (sumxaxis >> 2) & 0x3333333333333333;
    pop_high = (popcount >> 2) & 0x3333333333333333;
    sumxaxis = (sumxaxis & 0x3333333333333333) + sum_high;
    sumxaxis += pop_high << 1;
    popcount = (popcount & 0x3333333333333333) + pop_high;

    //  4 -> 8
    sum_high = (sumxaxis >> 4) & 0x0f0f0f0f0f0f0f0f;
    pop_high = (popcount >> 4) & 0x0f0f0f0f0f0f0f0f;
    sumxaxis = (sumxaxis & 0x0f0f0f0f0f0f0f0f) + sum_high;
    sumxaxis += pop_high << 2;
    popcount = (popcount & 0x0f0f0f0f0f0f0f0f) + pop_high;

    //  8 -> 16
    sum_high = (sumxaxis >> 8) & 0x00ff00ff00ff00ff;
    pop_high = (popcount >> 8) & 0x00ff00ff00ff00ff;
    sumxaxis = (sumxaxis & 0x00ff00ff00ff00ff) + sum_high;
    sumxaxis += pop_high << 3;
    popcount = (popcount & 0x00ff00ff00ff00ff) + pop_high;

    //  16 -> 32
    sum_high = (sumxaxis >> 16) & 0x0000ffff0000ffff;
    pop_high = (popcount >> 16) & 0x0000ffff0000ffff;
    sumxaxis = (sumxaxis & 0x0000ffff0000ffff) + sum_high;
    sumxaxis += pop_high << 4;
    popcount = (popcount & 0x0000ffff0000ffff) + pop_high;

    //  32 -> 64
    sum_high = sumxaxis >> 32;
    pop_high = popcount >> 32;
    sumxaxis += sum_high;
    sumxaxis += pop_high << 5;
    popcount += pop_high;

    sum_index = (uint32_t)sumxaxis;
    return (uint32_t)popcount;
}

static PA_FORCE_INLINE uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const Tile& tile
){
    static_assert(Tile::WIDTH == 64);

    uint64_t sum_p = 0;
    uint64_t sum_x = 0;
    uint64_t sum_y = 0;
    {
        uint64_t sum;
        sum_p += popcount_indexsum(sum, tile.row(0));
        sum_x += sum;
    }
    for (size_t c = 1; c < Tile::HEIGHT; c++){
        uint64_t pop, sum;
        pop = popcount_indexsum(sum, tile.row(c));
        sum_p += pop;
        sum_x += sum;
        sum_y += pop * c;
    }
    sum_xcoord = sum_x;
    sum_ycoord = sum_y;
    return sum_p;
}




//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static PA_FORCE_INLINE bool waterfill_touch_top(
    const Tile& mask, Tile& tile, const Tile& border
){
    uint64_t available = mask.row(0) & ~tile.row(0);
    uint64_t new_bits = available & border.row(Tile::HEIGHT - 1);
    if (new_bits == 0){
        return false;
    }
    tile.row(0) |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_bottom(
    const Tile& mask, Tile& tile, const Tile& border
){
    uint64_t available = mask.row(Tile::HEIGHT - 1) & ~tile.row(Tile::HEIGHT - 1);
    uint64_t new_bits = available & border.row(0);
    if (new_bits == 0){
        return false;
    }
    tile.row(Tile::HEIGHT - 1) |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_left(
    const Tile& mask, Tile& tile, const Tile& border
){
    bool changed = false;
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        uint64_t available = mask.row(c) & ~tile.row(c);
        uint64_t new_bits = available & (border.row(c) >> 63);
        changed |= new_bits != 0;
        tile.row(c) |= new_bits;
    }
    return changed;
}
static PA_FORCE_INLINE bool waterfill_touch_right(
    const Tile& mask, Tile& tile, const Tile& border
){
    bool changed = false;
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        uint64_t available = mask.row(c) & ~tile.row(c);
        uint64_t new_bits = available & (border.row(c) << 63);
        changed |= new_bits != 0;
        tile.row(c) |= new_bits;
    }
    return changed;
}





//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x". Clear bits of object from "m".
static PA_FORCE_INLINE void waterfill_expand(Tile& m, Tile& x){
    uint64_t xs[Tile::HEIGHT];
    for (size_t c = 0; c < Tile::HEIGHT; c++){
        xs[c] = x.row(c);
    }

    Waterfill_64xH_Default_ProcessedMask<Tile> mask(m, xs);
    expand_forward(mask, xs);

    uint64_t ms[Tile::HEIGHT];
    do{
        expand_vertical(mask, xs);
        expand_reverse(mask, xs);
        expand_forward(mask, xs);
    }while (keep_going(mask, ms, xs));

    for (size_t c = 0; c < Tile::HEIGHT; c++){
        x.row(c) = xs[c];
        m.row(c) = ms[c];
    }
}




};



}
}
}
#endif
