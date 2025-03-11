/*  Waterfill Core (Default)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x4_Default_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x4_Default_H

#include "Kernels_Waterfill_Core_64xH_Default.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


template <typename Tile>
struct Waterfill_64x4_Default_ProcessedMask{
    static_assert(Tile::WIDTH == 64);
    static_assert(Tile::HEIGHT == 4);

    uint64_t m0, m1, m2, m3; //  Copy of the masks.
    uint64_t b0, b1, b2, b3; //  Bit-reversed copy of the masks.

    PA_FORCE_INLINE Waterfill_64x4_Default_ProcessedMask(
        const Tile& m,
        uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3
    ){
        m0 = x0 | m.row(0);
        m1 = x1 | m.row(1);
        m2 = x2 | m.row(2);
        m3 = x3 | m.row(3);

        b0 = bitreverse64(m0);
        b1 = bitreverse64(m1);
        b2 = bitreverse64(m2);
        b3 = bitreverse64(m3);
    }
};




template <typename Tile>
PA_FORCE_INLINE bool keep_going(
    const Waterfill_64x4_Default_ProcessedMask<Tile>& mask,
    uint64_t& m0, uint64_t& m1, uint64_t& m2, uint64_t& m3,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    m0 = ~x0 & mask.m0;
    m1 = ~x1 & mask.m1;
    m2 = ~x2 & mask.m2;
    m3 = ~x3 & mask.m3;

    uint64_t changed =
               x0 & ((m0 << 1) |      m1);
    changed |= x1 & ((m1 << 1) | m0 | m2);
    changed |= x2 & ((m2 << 1) | m1 | m3);
    changed |= x3 & ((m3 << 1) | m2);

    return changed;
}



template <typename Tile>
PA_FORCE_INLINE void expand_forward(
    const Waterfill_64x4_Default_ProcessedMask<Tile>& mask,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    uint64_t s0 = x0 + mask.m0;
    uint64_t s1 = x1 + mask.m1;
    uint64_t s2 = x2 + mask.m2;
    uint64_t s3 = x3 + mask.m3;

    s0 ^= mask.m0;
    s1 ^= mask.m1;
    s2 ^= mask.m2;
    s3 ^= mask.m3;

    s0 &= mask.m0;
    s1 &= mask.m1;
    s2 &= mask.m2;
    s3 &= mask.m3;

    x0 |= s0;
    x1 |= s1;
    x2 |= s2;
    x3 |= s3;
}

PA_FORCE_INLINE void expand_reverse(uint64_t m, uint64_t b, uint64_t& x){
    uint64_t s = bitreverse64(bitreverse64(x) + b);
    s ^= m;
    s &= m;
    x |= s;
}

template <typename Tile>
PA_FORCE_INLINE void expand_reverse(
    const Waterfill_64x4_Default_ProcessedMask<Tile>& mask,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
}

template <typename Tile>
PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x4_Default_ProcessedMask<Tile>& mask,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    x1 |= x0 & mask.m1;
    x2 |= x3 & mask.m2;
    x2 |= x1 & mask.m2;
    x1 |= x2 & mask.m1;
    x3 |= x2 & mask.m3;
    x0 |= x1 & mask.m0;
}





template <typename Tile>
struct Waterfill_64x4_Default : public Waterfill_64xH_Default<Tile>{


//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x". Clear bits of object from "m".
static PA_FORCE_INLINE void waterfill_expand(Tile& m, Tile& x){
    uint64_t x0 = x.row(0);
    uint64_t x1 = x.row(1);
    uint64_t x2 = x.row(2);
    uint64_t x3 = x.row(3);

    Waterfill_64x4_Default_ProcessedMask<Tile> mask(m, x0, x1, x2, x3);
    expand_forward(mask, x0, x1, x2, x3);

    uint64_t m0, m1, m2, m3;
    do{
        expand_vertical(mask, x0, x1, x2, x3);
        expand_reverse(mask, x0, x1, x2, x3);
        expand_forward(mask, x0, x1, x2, x3);
    }while (keep_going(
        mask,
        m0, m1, m2, m3,
        x0, x1, x2, x3
    ));
    x.row(0) = x0;
    x.row(1) = x1;
    x.row(2) = x2;
    x.row(3) = x3;
    m.row(0) = m0;
    m.row(1) = m1;
    m.row(2) = m2;
    m.row(3) = m3;
}



};



}
}
}
#endif
