/*  Binary Matrix Tile (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryMatrixTile_64x8_arm64_NEON_H
#define PokemonAutomation_Kernels_BinaryMatrixTile_64x8_arm64_NEON_H

#include <arm_neon.h>
#include "Common/Compiler.h"
#include "Kernels_BinaryMatrixTile_Debugging.h"
#include "Kernels_BinaryMatrix.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace Kernels{


struct BinaryTile_64x8_arm64_NEON{
    static constexpr BinaryMatrixType TYPE = BinaryMatrixType::arm64x8_x64_NEON;
    static constexpr size_t WIDTH = 64;
    static constexpr size_t HEIGHT = 8;

    uint64x2x4_t vec;

public:
    PA_FORCE_INLINE BinaryTile_64x8_arm64_NEON(){
        set_zero();
    }
    PA_FORCE_INLINE BinaryTile_64x8_arm64_NEON(const BinaryTile_64x8_arm64_NEON& x){
        vec.val[0] = x.vec.val[0];
        vec.val[1] = x.vec.val[1];
        vec.val[2] = x.vec.val[2];
        vec.val[3] = x.vec.val[3];
    }
    PA_FORCE_INLINE void operator=(const BinaryTile_64x8_arm64_NEON& x){
        vec.val[0] = x.vec.val[0];
        vec.val[1] = x.vec.val[1];
        vec.val[2] = x.vec.val[2];
        vec.val[3] = x.vec.val[3];
    }


public:
    PA_FORCE_INLINE void set_zero(){
        vec.val[0] = vreinterpretq_u64_u8(vdupq_n_u8(0));
        vec.val[1] = vreinterpretq_u64_u8(vdupq_n_u8(0));
        vec.val[2] = vreinterpretq_u64_u8(vdupq_n_u8(0));
        vec.val[3] = vreinterpretq_u64_u8(vdupq_n_u8(0));
    }
    PA_FORCE_INLINE void set_ones(){
        vec.val[0] = vreinterpretq_u64_u8(vdupq_n_u8(0xff));
        vec.val[1] = vreinterpretq_u64_u8(vdupq_n_u8(0xff));
        vec.val[2] = vreinterpretq_u64_u8(vdupq_n_u8(0xff));
        vec.val[3] = vreinterpretq_u64_u8(vdupq_n_u8(0xff));
    }
    // Set a tile with partial 1 bits. The 1-bits are in a sub-block (0, 0, width, height).
    // The rest of the tile is filled with 0 bits.
    PA_FORCE_INLINE void set_ones(size_t width, size_t height){
        uint64x2_t word = vdupq_n_u64(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        uint64x2_t vheight = vdupq_n_u64(height);
        uint64x2_t mask;
        // If height > (0, 1)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(0), vcreate_u64(1))); // low, high
        vec.val[0] = vandq_u64(mask, word);
        // If height > (2, 3)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(2), vcreate_u64(3))); // low, high
        vec.val[1] = vandq_u64(mask, word);
        // If height > (4, 5)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(4), vcreate_u64(5))); // low, high
        vec.val[2] = vandq_u64(mask, word);
        // If height > (6, 7)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(6), vcreate_u64(7))); // low, high
        vec.val[3] = vandq_u64(mask, word);
    }
    // Retain the tile content in sub-block (0, 0, width, height), clear the rest with 0 bits.
    PA_FORCE_INLINE void clear_padding(size_t width, size_t height){
        uint64x2_t word = vdupq_n_u64(
            width < 64
                ? ((uint64_t)1 << width) - 1
                : 0xffffffffffffffff
        );
        uint64x2_t vheight = vdupq_n_u64(height);
        uint64x2_t mask;
        // If height > (0, 1)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(0), vcreate_u64(1))); // low, high
        vec.val[0] = vandq_u64(vec.val[0], vandq_u64(mask, word));
        // If height > (2, 3)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(2), vcreate_u64(3))); // low, high
        vec.val[1] = vandq_u64(vec.val[1], vandq_u64(mask, word));
        // If height > (4, 5)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(4), vcreate_u64(5))); // low, high
        vec.val[2] = vandq_u64(vec.val[2], vandq_u64(mask, word));
        // If height > (6, 7)
        mask = vcgtq_u64(vheight, vcombine_u64(vcreate_u64(6), vcreate_u64(7))); // low, high
        vec.val[3] = vandq_u64(vec.val[3], vandq_u64(mask, word));
    }
    // Invert all bits by logical XOR with all 1 bits
    PA_FORCE_INLINE void invert(){
        vec.val[0] = veorq_u64(vec.val[0], vreinterpretq_u64_u8(vdupq_n_u8(0xff)));
        vec.val[1] = veorq_u64(vec.val[1], vreinterpretq_u64_u8(vdupq_n_u8(0xff)));
        vec.val[2] = veorq_u64(vec.val[2], vreinterpretq_u64_u8(vdupq_n_u8(0xff)));
        vec.val[3] = veorq_u64(vec.val[3], vreinterpretq_u64_u8(vdupq_n_u8(0xff)));
    }
    // logical XOR assign
    PA_FORCE_INLINE void operator^=(const BinaryTile_64x8_arm64_NEON& x){
        vec.val[0] = veorq_u64(vec.val[0], x.vec.val[0]);
        vec.val[1] = veorq_u64(vec.val[1], x.vec.val[1]);
        vec.val[2] = veorq_u64(vec.val[2], x.vec.val[2]);
        vec.val[3] = veorq_u64(vec.val[3], x.vec.val[3]);
    }
    // logical OR assign
    PA_FORCE_INLINE void operator|=(const BinaryTile_64x8_arm64_NEON& x){
        vec.val[0] = vorrq_u64(vec.val[0], x.vec.val[0]);
        vec.val[1] = vorrq_u64(vec.val[1], x.vec.val[1]);
        vec.val[2] = vorrq_u64(vec.val[2], x.vec.val[2]);
        vec.val[3] = vorrq_u64(vec.val[3], x.vec.val[3]);
    }
    // logical AND assign
    PA_FORCE_INLINE void operator&=(const BinaryTile_64x8_arm64_NEON& x){
        vec.val[0] = vandq_u64(vec.val[0], x.vec.val[0]);
        vec.val[1] = vandq_u64(vec.val[1], x.vec.val[1]);
        vec.val[2] = vandq_u64(vec.val[2], x.vec.val[2]);
        vec.val[3] = vandq_u64(vec.val[3], x.vec.val[3]);
    }
    // this = (NOT x) AND this
    PA_FORCE_INLINE void andnot(const BinaryTile_64x8_arm64_NEON& x){
        // vbicq_u64(a, b): a AND (NOT b)
        vec.val[0] = vbicq_u64(vec.val[0], x.vec.val[0]);
        vec.val[1] = vbicq_u64(vec.val[1], x.vec.val[1]);
        vec.val[2] = vbicq_u64(vec.val[2], x.vec.val[2]);
        vec.val[3] = vbicq_u64(vec.val[3], x.vec.val[3]);
    }


public:
    PA_FORCE_INLINE uint64_t top() const{
        return vec.val[0][0];
    }
    PA_FORCE_INLINE uint64_t& top(){
        return ((uint64_t*)&vec.val[0])[0];
    }
    PA_FORCE_INLINE uint64_t bottom() const{
        return vec.val[3][1];
    }
    PA_FORCE_INLINE uint64_t& bottom(){
        return ((uint64_t*)&vec.val[3])[1];
    }

    PA_FORCE_INLINE uint64_t row(size_t index) const{
        return vec.val[index/2][index%2];
    }
    PA_FORCE_INLINE uint64_t& row(size_t index){
        return ((uint64_t*)&vec.val[index/2])[index%2];
    }

    // Slow function: get bit at coordinate (x, y), x: [0, width), y: [0, height)
    bool get_bit(size_t x, size_t y) const{
        return (row(y) >> x) & 1;
    }
    // Slow function: set bit at coordinate (x, y), x: [0, width), y: [0, height)
    void set_bit(size_t x, size_t y){
        row(y) |= (uint64_t)1 << x;
    }
    void set_bit(size_t x, size_t y, bool set){
        uint64_t bit = (uint64_t)(set ? 1 : 0) << x;
        uint64_t mask = (uint64_t)1 << x;
        uint64_t& word = row(y);
        word = (word & ~mask) | bit;
    }

    std::string dump() const{
        std::string str;
        for (size_t c = 0; c < 8; c++){
            str += dump64(row(c)) + "\n";
        }
        return str;
    }


public:
    // Copy part of this tile into `tile` while applying the specified shifts.
    // These are used to implement submatrix extraction where the desired
    // sub-matrix may have arbitrary shift and alignment.
    // The area covered by `tile` on the image is (+shift_x, +shift_y) from the area of this tile.
    //
    // The suffix "_pp" stands for positive (x), positive (y).
    // The shifted values are first performend a logical OR with the values in `tile` before assigned to `tile`.
    // In this way the operation does not damage other un-assigned regions on `tile`.
    void copy_to_shift_pp(BinaryTile_64x8_arm64_NEON& tile, size_t shift_x, size_t shift_y) const{
        int64x2_t neg_shift_x_u64x2 = vmovq_n_s64(-(int64_t)shift_x);
        const uint64_t* src = (const uint64_t*)&vec;
        uint64_t* dest = (uint64_t*)&tile.vec;
        while (shift_y < 7){
            uint64x2_t row_64x2 = vld1q_u64(src + shift_y);
            // NEON only has left shift with a variable. So have to left shift by a negative value
            // to create right shift.
            row_64x2 = vshlq_u64(row_64x2, neg_shift_x_u64x2);
            row_64x2 = vorrq_u64(row_64x2, vld1q_u64(dest));
            vst1q_u64(dest, row_64x2);
            dest += 2;
            shift_y += 2;
        }
        if (shift_y < 8){
            dest[0] |= src[shift_y] >> shift_x;
        }
    }
    // Copy part of this tile into `tile` while applying the specified shifts.
    // These are used to implement submatrix extraction where the desired
    // sub-matrix may have arbitrary shift and alignment.
    // The area covered by `tile` on the image is (-shift_x, +shift_y) from the area of this tile.
    //
    // The suffix "_np" stands for negative (x), positive (y).
    // The shifted values are first performend a logical OR with the values in `tile` before assigned to `tile`.
    // In this way it the operation does not damage other un-assigned regions on `tile`.
    void copy_to_shift_np(BinaryTile_64x8_arm64_NEON& tile, size_t shift_x, size_t shift_y) const{
        uint64x2_t shift_x_u64x2 = vdupq_n_u64(shift_x);
        const uint64_t* src = (const uint64_t*)&vec;
        uint64_t* dest = (uint64_t*)&tile.vec;
        while (shift_y < 7){
            uint64x2_t row_64x2 = vld1q_u64(src + shift_y);
            // left shift
            row_64x2 = vshlq_u64(row_64x2, shift_x_u64x2);
            row_64x2 = vorrq_u64(row_64x2, vld1q_u64(dest));
            vst1q_u64(dest, row_64x2);
            dest += 2;
            shift_y += 2;
        }
        if (shift_y < 8){
            dest[0] |= src[shift_y] << shift_x;
        }
    }
    // Copy part of this tile into `tile` while applying the specified shifts.
    // These are used to implement submatrix extraction where the desired
    // sub-matrix may have arbitrary shift and alignment.
    // The area covered by `tile` on the image is (+shift_x, -shift_y) from the area of this tile.
    //
    // The suffix "_pn" stands for positive (x), negative (y).
    // The shifted values are first performend a logical OR with the values in `tile` before assigned to `tile`.
    // In this way it the operation does not damage other un-assigned regions on `tile`.
    void copy_to_shift_pn(BinaryTile_64x8_arm64_NEON& tile, size_t shift_x, size_t shift_y) const{
        int64x2_t neg_shift_x_u64x2 = vmovq_n_s64(-(int64_t)shift_x);
        const uint64_t* src = (const uint64_t*)&vec;
        uint64_t* dest = (uint64_t*)&tile.vec;
        if (shift_y & 1){
            dest[shift_y] |= src[0] >> shift_x;
            src++;
            shift_y++;
        }
        while (shift_y < 8){
            uint64x2_t row_64x2 = vld1q_u64(src);
            // NEON only has left shift with a variable. So have to left shift by a negative value
            // to create right shift.
            row_64x2 = vshlq_u64(row_64x2, neg_shift_x_u64x2);
            row_64x2 = vorrq_u64(row_64x2, vld1q_u64(dest + shift_y));
            vst1q_u64((dest + shift_y), row_64x2);
            src += 2;
            shift_y += 2;
        }
    }
    // Copy part of this tile into `tile` while applying the specified shifts.
    // These are used to implement submatrix extraction where the desired
    // sub-matrix may have arbitrary shift and alignment.
    // The area covered by `tile` on the image is (-shift_x, -shift_y) from the area of this tile.
    //
    // The suffix "_nn" stands for negative (x), negative (y).
    // The shifted values are first performend a logical OR with the values in `tile` before assigned to `tile`.
    // In this way it the operation does not damage other un-assigned regions on `tile`.
    void copy_to_shift_nn(BinaryTile_64x8_arm64_NEON& tile, size_t shift_x, size_t shift_y) const{
        uint64x2_t shift_x_u64x2 = vdupq_n_u64(shift_x);
        const uint64_t* src = (const uint64_t*)&vec;
        uint64_t* dest = (uint64_t*)&tile.vec;
        if (shift_y & 1){
            dest[shift_y] |= src[0] << shift_x;
            src++;
            shift_y++;
        }
        while (shift_y < 8){
            uint64x2_t row_64x2 = vld1q_u64(src);
            // left shift
            row_64x2 = vshlq_u64(row_64x2, shift_x_u64x2);
            row_64x2 = vorrq_u64(row_64x2, vld1q_u64(dest + shift_y));
            vst1q_u64((dest + shift_y), row_64x2);
            src += 2;
            shift_y += 2;
        }
    }
};





}
}
#endif
