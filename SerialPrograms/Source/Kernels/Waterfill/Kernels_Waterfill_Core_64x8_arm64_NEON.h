/*  Waterfill Core (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Waterfill_Core_64x8_arm64_NEON_H
#define PokemonAutomation_Kernels_Waterfill_Core_64x8_arm64_NEON_H

#include "Kernels/Kernels_BitScan.h"
#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels/BinaryMatrix/Kernels_BinaryMatrix_Arch_64x8_arm64_NEON.h"

namespace PokemonAutomation{
namespace Kernels{
namespace Waterfill{


PA_FORCE_INLINE uint64x2_t bit_reverse(uint64x2_t x){
    uint8x16_t r0, r1;

    // Use byte table lookup to swap the endianness of the two uint64_t
    uint8x16_t x_shuffled = vqtbl1q_u8(vreinterpretq_u8_u64(x), uint8x16_t{7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8});

    // If originaly one byte in `x` has the order (high to low) h, g, f, e, d, c, b, a:
    // right shift each byte by 4, r0 has (high to low) 0, 0, 0, 0, h, g, f, e
    r0 = vshrq_n_u8(x_shuffled, 4);
    // left shift each byte by 4, r1 has (high to low) d, c, b, a, 0, 0, 0, 0
    r1 = vshlq_n_u8(x_shuffled, 4);
    // after this logical OR in `r1` it is now (high to low) d, c, b, a, h, g, f, e
    r1 = vorrq_u8(r0, r1);

    // after right shift, r0 now has 0, 0, d, c, b, a, h, g
    r0 = vshrq_n_u8(r1, 2);
    // after left shif,t r1 now has b, a, h, g, f, e, 0, 0
    r1 = vshlq_n_u8(r1, 2);

    // After logical AND with 0x00110011, r0 has 0, 0, d, c, 0, 0, h, g
    r0 = vandq_u8(r0, vdupq_n_u8(0x33));
    // After logical AND with 0x11001100, r0 has b, a, 0, 0, f, e, 0, 0
    r1 = vandq_u8(r1, vdupq_n_u8(0xcc));
    // after logical OR r1 has b, a, d, c, f, e, h, g
    r1 = vorrq_u8(r0, r1);

    r0 = vshrq_n_u8(r1, 1); // 0, b, a, d, c, f, e, h
    r1 = vshlq_n_u8(r1, 1); // a, d, c, f, e, h, g, 0
    // logical AND with 0x01010101, r0 has 0, b, 0, d, 0, f, 0, h
    r0 = vandq_u8(r0, vdupq_n_u8(0x55));
    // logical AND with 0x10101010, r1 has a, 0, c, 0, e, 0, g, 0
    r1 = vandq_u8(r1, vdupq_n_u8(0xaa));
    r1 = vorrq_u8(r0, r1); // a, b, c, d, e, f, g, h

    return r1;
}



struct Waterfill_64x8_arm64_NEON_ProcessedMask{
    uint64x2_t m0, m1, m2, m3; //  Copy of logical OR of the mask `m` and recorded tile `x` bits 
    uint64x2_t b0, b1, b2, b3; //  Bit-reversed copy of m0, m1, m2, m3.
    uint64x2_t t0, t1, t2, t3; //  Transposed masks.
    uint64x2_t f1, f2, f3;     //  Forward-carry mask.
    uint64x2_t r0, r1, r2;     //  Reverse-carry mask.

    PA_FORCE_INLINE Waterfill_64x8_arm64_NEON_ProcessedMask(
        const BinaryTile_64x8_arm64_NEON& m,
        uint64x2_t x0, uint64x2_t x1, uint64x2_t x2, uint64x2_t x3
    ){
        m0 = vorrq_u64(x0, m.vec.val[0]);
        m1 = vorrq_u64(x1, m.vec.val[1]);
        m2 = vorrq_u64(x2, m.vec.val[2]);
        m3 = vorrq_u64(x3, m.vec.val[3]);

        b0 = bit_reverse(m0);
        b1 = bit_reverse(m1);
        b2 = bit_reverse(m2);
        b3 = bit_reverse(m3);

        t0 = m0;
        t1 = m1;
        t2 = m2;
        t3 = m3;
        transpose_u64_2x2_NEON(t0, t1);
        transpose_u64_2x2_NEON(t2, t3);

        //  Forward carry
        uint64x2_t f0 = t0;
        f1 = vandq_u64(f0, t1);
        transpose_u64_2x2_NEON(f0, f1);
        f2 = t2;
        f3 = vandq_u64(f2, t3);
        transpose_u64_2x2_NEON(f2, f3);

        //  Reverse carry
        uint64x2_t r3 = t3;
        r2 = vandq_u64(r3, t2);
        transpose_u64_2x2_NEON(r2, r3);
        r1 = t1;
        r0 = vandq_u64(r1, t0);
        transpose_u64_2x2_NEON(r0, r1);
    }
};



PA_FORCE_INLINE bool keep_going(
    const Waterfill_64x8_arm64_NEON_ProcessedMask& mask,
    uint64x2_t& m0, uint64x2_t& m1, uint64x2_t& m2, uint64x2_t& m3,
    uint64x2_t& x0, uint64x2_t& x1, uint64x2_t& x2, uint64x2_t& x3
){
    m0 = vbicq_u64(mask.m0, x0);
    m1 = vbicq_u64(mask.m1, x1);
    m2 = vbicq_u64(mask.m2, x2);
    m3 = vbicq_u64(mask.m3, x3);

    uint64x2_t r0;

    r0 = vshlq_n_u64(m0, 1);
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m0), vget_low_u64(m1)));
    r0 = vorrq_u64(r0, vcombine_u64(vcreate_u64(0), vget_low_u64(m0)));
    uint64x2_t changed = vandq_u64(r0, x0);

    r0 = vshlq_n_u64(m1, 1);
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m1), vget_low_u64(m2)));
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m0), vget_low_u64(m1)));
    r0 = vandq_u64(r0, x1);
    changed = vorrq_u64(changed, r0);

    r0 = vshlq_n_u64(m2, 1);
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m2), vget_low_u64(m3)));
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m1), vget_low_u64(m2)));
    r0 = vandq_u64(r0, x2);
    changed = vorrq_u64(changed, r0);

    r0 = vshlq_n_u64(m3, 1);
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m3), vcreate_u64(0)));
    r0 = vorrq_u64(r0, vcombine_u64(vget_high_u64(m2), vget_low_u64(m3)));
    r0 = vandq_u64(r0, x3);
    changed = vorrq_u64(changed, r0);

    // Test if there is 1 bit in `changed`
    return vgetq_lane_u64(changed, 0) | vgetq_lane_u64(changed, 1);
}



PA_FORCE_INLINE void expand_forward(
    const Waterfill_64x8_arm64_NEON_ProcessedMask& mask,
    uint64x2_t& x0, uint64x2_t& x1, uint64x2_t& x2, uint64x2_t& x3
){
    uint64x2_t s0 = vaddq_u64(x0, mask.m0);
    uint64x2_t s1 = vaddq_u64(x1, mask.m1);
    uint64x2_t s2 = vaddq_u64(x2, mask.m2);
    uint64x2_t s3 = vaddq_u64(x3, mask.m3);

    s0 = vbicq_u64(mask.m0, s0);
    s1 = vbicq_u64(mask.m1, s1);
    s2 = vbicq_u64(mask.m2, s2);
    s3 = vbicq_u64(mask.m3, s3);

    x0 = vorrq_u64(x0, s0);
    x1 = vorrq_u64(x1, s1);
    x2 = vorrq_u64(x2, s2);
    x3 = vorrq_u64(x3, s3);
}
PA_FORCE_INLINE void expand_reverse(uint64x2_t m, uint64x2_t b, uint64x2_t& x){
    uint64x2_t s = bit_reverse(vaddq_u64(bit_reverse(x), b));
    s = veorq_u64(s, m);
    s = vandq_u64(s, m);
    x = vorrq_u64(x, s);
}
PA_FORCE_INLINE void expand_reverse(
    const Waterfill_64x8_arm64_NEON_ProcessedMask& mask,
    uint64x2_t& x0, uint64x2_t& x1, uint64x2_t& x2, uint64x2_t& x3
){
    expand_reverse(mask.m0, mask.b0, x0);
    expand_reverse(mask.m1, mask.b1, x1);
    expand_reverse(mask.m2, mask.b2, x2);
    expand_reverse(mask.m3, mask.b3, x3);
}
PA_FORCE_INLINE void expand_vertical(
    const Waterfill_64x8_arm64_NEON_ProcessedMask& mask,
    uint64x2_t& x0, uint64x2_t& x1, uint64x2_t& x2, uint64x2_t& x3
){
    //  Carry across adjacent rows.
    transpose_u64_2x2_NEON(x0, x1);
    transpose_u64_2x2_NEON(x2, x3);
    x0 = vorrq_u64(x0, vandq_u64(x1, mask.t0));
    x1 = vorrq_u64(x1, vandq_u64(x0, mask.t1));
    x2 = vorrq_u64(x2, vandq_u64(x3, mask.t2));
    x3 = vorrq_u64(x3, vandq_u64(x2, mask.t3));
    transpose_u64_2x2_NEON(x0, x1);
    transpose_u64_2x2_NEON(x2, x3);

    //  Carry across groups of 2 rows.
    uint64x1_t x0h = vget_high_u64(x0);
    x1 = vorrq_u64(x1, vandq_u64(vcombine_u64(x0h, x0h), mask.f1));
    uint64x1_t x3l = vget_low_u64(x3);
    x2 = vorrq_u64(x2, vandq_u64(vcombine_u64(x3l, x3l), mask.r2));
    uint64x1_t x1h = vget_high_u64(x1);
    x2 = vorrq_u64(x2, vandq_u64(vcombine_u64(x1h, x1h), mask.f2));
    uint64x1_t x2l = vget_low_u64(x2);
    x1 = vorrq_u64(x1, vandq_u64(vcombine_u64(x2l, x2l), mask.r1));
    uint64x1_t x2h = vget_high_u64(x2);
    x3 = vorrq_u64(x3, vandq_u64(vcombine_u64(x2h, x2h), mask.f3));
    uint64x1_t x1l = vget_low_u64(x1);
    x0 = vorrq_u64(x0, vandq_u64(vcombine_u64(x1l, x1l), mask.r0));
}



struct Waterfill_64x8_arm64_NEON{



// Do vector-wise logical OR among each SIMD vector
static PA_FORCE_INLINE uint64x2_t vec_or(const BinaryTile_64x8_arm64_NEON& tile){
    uint64x2_t v0 = vorrq_u64(tile.vec.val[0], tile.vec.val[1]);
    uint64x2_t v1 = vorrq_u64(tile.vec.val[2], tile.vec.val[3]);
    return vorrq_u64(v0, v1);
}
// Do a in-tile row-wise logical OR among each row
static PA_FORCE_INLINE uint64_t row_or(const BinaryTile_64x8_arm64_NEON& tile){
    uint64x2_t v = vec_or(tile);
    return vgetq_lane_u64(v, 0) | vgetq_lane_u64(v, 1);
}


//  Find a one bit in the specified tile.
//  If found, (x, y) are set to its coordinates and returns true.
//  If entire tile is zero, returns false.
static PA_FORCE_INLINE bool find_bit(size_t& x, size_t& y, const BinaryTile_64x8_arm64_NEON& tile){
    // Check whether thera are any 1-bits in the tile:
    uint64x2_t anything = vec_or(tile);
    if ((vgetq_lane_u64(anything, 0) | vgetq_lane_u64(anything, 1)) == 0){
        return false;
    }
    // Check each tile row:
    for (size_t r = 0; r < 8; r++){
        size_t pos;
        if (trailing_zeros(pos, tile.row(r))){
            x = pos;
            y = r;
            return true;
        }
    }
    return false;
}



//  Finds the boundaries of the one-bits inside the tile.
//  Max values are one past the end.
//  Behavior is undefined if tile is zero.
static PA_FORCE_INLINE void boundaries(
    const BinaryTile_64x8_arm64_NEON& tile,
    size_t& min_x, size_t& max_x,
    size_t& min_y, size_t& max_y
){
    uint64_t all_or = row_or(tile);
    trailing_zeros(min_x, all_or);
    max_x = bitlength(all_or);

    min_y = 0;
    for (size_t c = 0; c < 8; c++){
        if (tile.row(c) != 0){
            min_y = c;
            break;
        }
    }
    max_y = 0;
    for (size_t c = 8; c > 0; c--){
        if (tile.row(c - 1) != 0){
            max_y = c;
            break;
        }
    }
}



// Compute the sum of the indices of 1 bits in `x`  and store in `sum_index`, and compute
// counts of 1 bits (aka popcount) of `x` and return it as the function return.
// Note: `sum_index` is useful for computing center of gravity of bitmap objects.
// Example of index sum of a bitmap:
// 0b0001 is 0
// 0b0100 is 2
// 0b1010 is 1 + 3 = 4
static PA_FORCE_INLINE uint64x2_t popcount_indexsum(uint64x2_t& sum_index, uint64x2_t x){
    //  1 -> 2
    // Get the bits in the original [0, 2, 4, 6, ...] bit positions by right shifting by one, then
    // logical AND with 0b ... 0101 0101
    uint8x16_t pop_high = vandq_u8(vshrq_n_u8(vreinterpretq_u8_u64(x), 1), vdupq_n_u8(0x55));
    // Get the bits in the original [1, 3, 5, 7, ...] bit positions by logical AND with 0b ... 0101 0101
    uint8x16_t pop_low = vandq_u8(vreinterpretq_u8_u64(x), vdupq_n_u8(0x55));
    // Add them to get the bit-pair-wise addition
    // Each nbring pair of bits is added in the pair and the result is stored in a 2-bit-wide space.
    // The result can only be 0b00, 0b01 or 0b10 so it won't overflow the 2-bit storage.
    // You can think `popcount` as storing the bit count of each 2-bit-wide space of the original bitmap
    uint8x16_t popcount = vaddq_u8(pop_low, pop_high);

    uint8x16_t sum_high, sum_low;
    // think `sumaxis` as containing the index sum of the bits of the 2-bit-wide space in the original bitmap,
    // For each 2-bit wide space in the original bitmap:
    // if it's 0b00, then index sum is 0 
    // if it's 0b01, then index sum is 0 (the 1 is at 0-th index, so the index sum is still 0)
    // if it's 0b10, then index sum is 1
    // if it's 0b11, then index sum is 1
    uint8x16_t sumxaxis = pop_high;

    //  2 -> 4
    // Right shift `sumxaxis` by 2, then logical AND with 0b ... 0011 0011
    // This means we get the 0b1100 part of `sumxaxis` into sum_high
    sum_high = vandq_u8(vshrq_n_u8(sumxaxis, 2), vdupq_n_u8(0x33));
    // Right shift `popcount` by 2, then logical AND with 0b ... 0011 0011
    // This means we get the 0b1100 part of `popcount` into `pop_high`
    pop_high = vandq_u8(vshrq_n_u8(popcount, 2), vdupq_n_u8(0x33));
    // logical AND of `sumxaxis` and 0b ... 0011 0011
    // This means we get the 0b0011 part of `sumxaxis` into `sum_low`
    sum_low = vandq_u8(sumxaxis, vdupq_n_u8(0x33));
    // Add sum_low and sum_high together for each 4-bit-wide space
    // Then add the count from `pop_high` shifted.
    // You can think index sum is made by "lower part" of a 4-bit-wide space and "higher part" of the space.
    // According to the physics reasoning that computes the combined center of gravity of a collection of objects,
    // 4-bit index sum = 2-bit index sum of the lower part + 2-bit index sum of the higher part
    //                 + bit count of the higher part * 2
    // `sumxaxis` value for each 4-bit-wide space ranges [0, 6]
    sumxaxis = vaddq_u8(sum_low, sum_high);
    sumxaxis = vaddq_u8(sumxaxis, vshlq_n_u8(pop_high, 1));

    // Get the 0b0011 part of `popcount` into `pop_low`
    pop_low = vandq_u8(popcount, vdupq_n_u8(0x33));
    // Add `pop_low` and `pop_high` together to count the bits in 4-bit-wide nbrhood in the original bitmap
    // The max value possible is 4 (aka 0b0100), so won't overflow 4-bit-wide space
    // You can think `popcount` as storing the bit count of each 4-bit-wide space of the original bitmap
    // You can think `pop_low` is the bit count of the "lower part" (in this block of compuattion, the lower
    // part of the 4-bit-wide space), while `pop_high` is the bit count of the "higher part".
    // `popcount` value for each 4-bit-wide space ranges [0, 4]
    popcount = vaddq_u8(pop_low, pop_high);

    //  4 -> 8
    // `sum_high`: higher part of the 8-bit-wide space in `sumxaxis`
    sum_high = vandq_u8(vshrq_n_u8(sumxaxis, 4), vdupq_n_u8(0x0f));
    // `pip_high`: higher part of the 8-bit-wide space in `popcount`
    pop_high = vandq_u8(vshrq_n_u8(popcount, 4), vdupq_n_u8(0x0f));
    // `sum_low`: lower part of the 8-bit-wide space in `sumxaxis`
    sum_low = vandq_u8(sumxaxis, vdupq_n_u8(0x0f));
    // `sumxaxis`: index sum for each 8-bit-wide space, value range: [0, 28]
    sumxaxis = vaddq_u8(sum_low, sum_high);
    sumxaxis = vaddq_u8(sumxaxis, vshlq_n_u8(pop_high, 2));
    // `pop_low`: lower part of the 8-bit-wide space in `popcount`
    pop_low = vandq_u8(popcount, vdupq_n_u8(0x0f));
    // `popcount` of the 8-bit-wide space, range [0, 8]
    popcount = vaddq_u8(pop_low, pop_high);

    //  8 -> 16
    // Get higher part of 16-bit-wide space in `popcount`
    uint16x8_t pop_high_u16x8 = vshrq_n_u16(vreinterpretq_u16_u8(popcount), 8);
    // Get the index sum for each 16-bit-wise space
    // vpadalq_u8(a, b): pair-wise add in `b`, then add to `a`
    // index sum range: [0, 120]
    uint16x8_t sumxaxis_u16x8 = vpadalq_u8(vshlq_n_u16(pop_high_u16x8, 3), sumxaxis);
    // Get popcount of each 16-bit-wise space
    // popcount value: [0, 16]
    uint16x8_t popcount_u16x8 = vpaddlq_u8(popcount);

    //  16 -> 32
    uint32x4_t pop_high_u32x4 = vshrq_n_u32(vreinterpretq_u32_u16(popcount_u16x8), 16);
    // Get the index sum for each 32-bit-wise space
    // index sum range: [0, 496]
    uint32x4_t sumxaxis_u32x4 = vpadalq_u16(vshlq_n_u32(pop_high_u32x4, 4), sumxaxis_u16x8);
    // Get popcount of each 32-bit-wise space
    // popcount value: [0, 32]
    uint32x4_t popcount_u32x4 = vpaddlq_u16(popcount_u16x8);

    //  32 -> 64
    uint64x2_t pop_high_u64x2 = vshrq_n_u64(vreinterpretq_u64_u32(popcount_u32x4), 32);
    // Get the index sum for each 64-bit-wise space
    uint64x2_t sumxaxis_u64x2 = vpadalq_u32(vshlq_n_u64(pop_high_u64x2, 5), sumxaxis_u32x4);
    // Get popcount of each 64-bit-wise space
    uint64x2_t popcount_u64x2 = vpaddlq_u32(popcount_u32x4);

    // TODO: The above vpadxlq_uxx() operations are horizontal, may be slow.
    // Can try to compare with SSE42 impl.

    sum_index = sumxaxis_u64x2;
    return popcount_u64x2;
}

static PA_FORCE_INLINE uint64_t popcount_sumcoord(
    uint64_t& sum_xcoord, uint64_t& sum_ycoord,
    const BinaryTile_64x8_arm64_NEON& tile
){
    uint64x2_t sum_p, sum_x, sum_y;
    {
        uint64x2_t pop, sum;
        pop = popcount_indexsum(sum, tile.vec.val[0]);
        sum_p = pop;
        sum_x = sum;
        sum_y = vmull_u32(vmovn_u64(pop), uint32x2_t{0, 1});
    }
    {
        uint64x2_t pop, sum;
        pop = popcount_indexsum(sum, tile.vec.val[1]);
        sum_p = vaddq_u64(sum_p, pop);
        sum_x = vaddq_u64(sum_x, sum);
        sum_y = vaddq_u64(sum_y, vmull_u32(vmovn_u64(pop), uint32x2_t{2, 3}));
    }
    {
        uint64x2_t pop, sum;
        pop = popcount_indexsum(sum, tile.vec.val[2]);
        sum_p = vaddq_u64(sum_p, pop);
        sum_x = vaddq_u64(sum_x, sum);
        sum_y = vaddq_u64(sum_y, vmull_u32(vmovn_u64(pop), uint32x2_t{4, 5}));
    }
    {
        uint64x2_t pop, sum;
        pop = popcount_indexsum(sum, tile.vec.val[3]);
        sum_p = vaddq_u64(sum_p, pop);
        sum_x = vaddq_u64(sum_x, sum);
        sum_y = vaddq_u64(sum_y, vmull_u32(vmovn_u64(pop), uint32x2_t{6, 7}));
    }
    sum_xcoord = vgetq_lane_u64(sum_x, 0) + vgetq_lane_u64(sum_x, 1);
    sum_ycoord = vgetq_lane_u64(sum_y, 0) + vgetq_lane_u64(sum_y, 1);
    return vgetq_lane_u64(sum_p, 0) + vgetq_lane_u64(sum_p, 1);
}



//  Run Waterfill algorithm on mask "m" with starting point "x".
//  Save result back into "x". Clear bits of object from "m".
static PA_FORCE_INLINE void waterfill_expand(BinaryTile_64x8_arm64_NEON& m, BinaryTile_64x8_arm64_NEON& x){
    uint64x2_t x0 = x.vec.val[0];
    uint64x2_t x1 = x.vec.val[1];
    uint64x2_t x2 = x.vec.val[2];
    uint64x2_t x3 = x.vec.val[3];

    Waterfill_64x8_arm64_NEON_ProcessedMask mask(m, x0, x1, x2, x3);
    expand_forward(mask, x0, x1, x2, x3);

    uint64x2_t m0, m1, m2, m3;
    do{
        expand_vertical(mask, x0, x1, x2, x3);
        expand_reverse(mask, x0, x1, x2, x3);
        expand_forward(mask, x0, x1, x2, x3);
    }while (keep_going(
        mask,
        m0, m1, m2, m3,
        x0, x1, x2, x3
    ));
    x.vec.val[0] = x0;
    x.vec.val[1] = x1;
    x.vec.val[2] = x2;
    x.vec.val[3] = x3;
    m.vec.val[0] = m0;
    m.vec.val[1] = m1;
    m.vec.val[2] = m2;
    m.vec.val[3] = m3;
}



//  Touch the edge of "tile" with the specified border.
//  Returns true if "tile" has changed and needs to be updated.
static PA_FORCE_INLINE bool waterfill_touch_top(
    const BinaryTile_64x8_arm64_NEON& mask,
    BinaryTile_64x8_arm64_NEON& tile,
    const BinaryTile_64x8_arm64_NEON& border
){
    uint64_t available = mask.top() & ~tile.top();
    uint64_t new_bits = available & border.bottom();
    if (new_bits == 0){
        return false;
    }
    tile.top() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_bottom(
    const BinaryTile_64x8_arm64_NEON& mask,
    BinaryTile_64x8_arm64_NEON& tile,
    const BinaryTile_64x8_arm64_NEON& border
){
    uint64_t available = mask.bottom() & ~tile.bottom();
    uint64_t new_bits = available & border.top();
    if (new_bits == 0){
        return false;
    }
    tile.bottom() |= new_bits;
    return true;
}
static PA_FORCE_INLINE bool waterfill_touch_left(
    const BinaryTile_64x8_arm64_NEON& mask,
    BinaryTile_64x8_arm64_NEON& tile,
    const BinaryTile_64x8_arm64_NEON& border
){
    uint64x2_t changed = vreinterpretq_u64_u8(vdupq_n_u8(0));
    for (size_t c = 0; c < 4; c++){
        uint64x2_t available = vbicq_u64(mask.vec.val[c], tile.vec.val[c]);
        uint64x2_t new_bits = vandq_u64(available, vshrq_n_u64(border.vec.val[c], 63));
        changed = vorrq_u64(changed, new_bits);
        tile.vec.val[c] = vorrq_u64(tile.vec.val[c], new_bits);
    }
    // Test if there is 1 bit in `changed`
    return vgetq_lane_u64(changed, 0) | vgetq_lane_u64(changed, 1);
}
static PA_FORCE_INLINE bool waterfill_touch_right(
    const BinaryTile_64x8_arm64_NEON& mask,
    BinaryTile_64x8_arm64_NEON& tile,
    const BinaryTile_64x8_arm64_NEON& border
){
    uint64x2_t changed = vreinterpretq_u64_u8(vdupq_n_u8(0));
    for (size_t c = 0; c < 4; c++){
        uint64x2_t available = vbicq_u64(mask.vec.val[c], tile.vec.val[c]);
        uint64x2_t new_bits = vandq_u64(available, vshlq_n_u64(border.vec.val[c], 63));
        changed = vorrq_u64(changed, new_bits);
        tile.vec.val[c] = vorrq_u64(tile.vec.val[c], new_bits);
    }
    // Test if there is 1 bit in `changed`
    return vgetq_lane_u64(changed, 0) | vgetq_lane_u64(changed, 1);
}


};



}
}
}
#endif
