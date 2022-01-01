/*  Waterfill Tile (64-bit integer)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_Waterfill_Tile_Default.h"

namespace PokemonAutomation{
namespace Kernels{


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


struct ProcessedMask{
    uint64_t m0, m1, m2, m3; //  Copy of the masks.
    uint64_t b0, b1, b2, b3; //  Bit-reversed copy of the masks.

    PA_FORCE_INLINE ProcessedMask(
        const BinaryTile_Default& m,
        uint64_t x0, uint64_t x1, uint64_t x2, uint64_t x3
    ){
        m0 = x0 | m.vec[0];
        m1 = x1 | m.vec[1];
        m2 = x2 | m.vec[2];
        m3 = x3 | m.vec[3];

        b0 = bitreverse64(m0);
        b1 = bitreverse64(m1);
        b2 = bitreverse64(m2);
        b3 = bitreverse64(m3);
    }
};


PA_FORCE_INLINE void expand_reverse(uint64_t m, uint64_t b, uint64_t& x){
    uint64_t s = bitreverse64(bitreverse64(x) + b);
    s ^= m;
    s &= m;
    x |= s;
}


PA_FORCE_INLINE void expand_forward(
    const ProcessedMask& mask,
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
PA_FORCE_INLINE void expand_reverse(
    const ProcessedMask& mask,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
}
PA_FORCE_INLINE void expand_vertical(
    const ProcessedMask& mask,
    uint64_t& x0, uint64_t& x1, uint64_t& x2, uint64_t& x3
){
    x1 |= x0 & mask.m1;
    x2 |= x3 & mask.m2;
    x2 |= x1 & mask.m2;
    x1 |= x2 & mask.m1;
    x3 |= x2 & mask.m3;
    x0 |= x1 & mask.m0;
}

void waterfill_expand(const BinaryTile_Default& m, BinaryTile_Default& x){
    uint64_t x0 = x.vec[0];
    uint64_t x1 = x.vec[1];
    uint64_t x2 = x.vec[2];
    uint64_t x3 = x.vec[3];

    ProcessedMask mask(m, x0, x1, x2, x3);

    uint64_t changed;
    do{
        expand_forward(mask, x0, x1, x2, x3);
        expand_vertical(mask, x0, x1, x2, x3);
        expand_reverse(mask, x0, x1, x2, x3);
        changed  = x0 ^ x.vec[0];
        changed |= x1 ^ x.vec[1];
        changed |= x2 ^ x.vec[2];
        changed |= x3 ^ x.vec[3];
        x.vec[0] = x0;
        x.vec[1] = x1;
        x.vec[2] = x2;
        x.vec[3] = x3;
//        cout << x.dump() << endl;
    }while (changed);
}







}
}
