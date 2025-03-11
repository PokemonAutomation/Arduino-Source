/*  Binary Image Basic Filters (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_arm64_NEON_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_arm64_NEON_H

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{
namespace Kernels{

// Change color of an array of pixels based on values from a bitmap that corresponds to the pixel array.
// If `replace_zero_bits` is true, change color of pixels that correspond to 0 bits.
// Otherwise, chagne color of pixels that correspond to 1 bits.
class FilterByMask_arm64_NEON{
public:
    FilterByMask_arm64_NEON(uint32_t replacement, bool replace_zero_bits)
        : m_replacement_u32(vdupq_n_u32(replacement))
        , m_replace_if_zero(replace_zero_bits)
        , m_zeros(vreinterpretq_u32_u8(vdupq_n_u8(0)))
    {}

    // Given 64 bits stored in `uint64_t`, use it to set colors to 64 pixels in `pixels`.
    // If filter constructor parameter `replace_zero_bits` is true, the pixels corresponding to
    // 0-bits are set to color `replacement` (another filter constructor parameter).
    // Otherwise, pixels corresponding to 1-bits are set to the color.
    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels) const{
        for(int i = 0; i < 64; i+=16){
            filter16((bits >> i) & 0xFFFF, pixels + i);
        }
    }

    // partial version of filter64(bits, pixels): instead of setting colors to 64 pixels,
    // only setting `count` (count <= 64) pixels.
    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count) const{
        const size_t count_round_4 = (count / 4) * 4;
        for(size_t i = 0; i < count_round_4; i+=4){
            filter4((bits >> i) & 0xF, pixels + i);
        }
        
        size_t left = count % 4;
        if (left){
            uint32_t buffer[4];
            memcpy(buffer, pixels + count_round_4, sizeof(uint32_t) * left);
            filter4((bits >> count_round_4) & 0xF, buffer);
            memcpy(pixels + count_round_4, buffer, sizeof(uint32_t) * left);
        }
    }

private:
    // Change color in the 8 pixels according to the lowest 4 bits in `bits`
    PA_FORCE_INLINE void filter4(uint64_t bits64, uint32_t* pixels) const{
        // Duplicate 4-bit pattern into four uint16_t places in `bits`
        bits64 *= 0x0001000100010001;
        // convert each uint16_t to be one bit from the lowest four bits in input `bits`
        bits64 &= 0x0008000400020001;

        uint32x4_t pixels_u32 = vld1q_u32(pixels);

        // Load `bits` into simd 64-bit vector register
        uint16x4_t mask_u16 = vcreate_u16(bits64);
        // Expand mask to cover each pixel (uint32_t)
        uint32x4_t mask_u32 = vmovl_u16(mask_u16);
        // Expand mask to be all-1 or all-0 mask for each pixel
        mask_u32 = vcgtq_u32(mask_u32, m_zeros);

        uint32x4_t out_u32;
        if (m_replace_if_zero){
            // bit select intrinsic: 
            // vbslq_u32(a, b, c), for 1 bits in a, choose b; for 0 bits in a, choose c
            out_u32 = vbslq_u32(mask_u32, pixels_u32, m_replacement_u32);
        }else{
            out_u32 = vbslq_u32(mask_u32, m_replacement_u32, pixels_u32);
        }
        vst1q_u32(pixels, out_u32);
    }

    PA_FORCE_INLINE void filter16(uint64_t bits64, uint32_t* pixels) const{
        uint64_t bits_0 = bits64 & 0xF;
        uint64_t bits_1 = (bits64 >> 4) & 0xF;
        uint64_t bits_2 = (bits64 >> 8) & 0xF;
        uint64_t bits_3 = bits64 >> 12;

        // Duplicate 4-bit pattern into four uint16_t places in `bits`
        bits_0 *= 0x0001000100010001;
        // convert each uint16_t to be one bit from the lowest four bits in input `bits`
        bits_0 &= 0x0008000400020001;
        
        bits_1 *= 0x0001000100010001;
        bits_1 &= 0x0008000400020001;

        bits_2 *= 0x0001000100010001;
        bits_2 &= 0x0008000400020001;

        bits_3 *= 0x0001000100010001;
        bits_3 &= 0x0008000400020001;

        uint32x4_t pixels_0_u32 = vld1q_u32(pixels);
        uint32x4_t pixels_1_u32 = vld1q_u32(pixels + 4);
        uint32x4_t pixels_2_u32 = vld1q_u32(pixels + 8);
        uint32x4_t pixels_3_u32 = vld1q_u32(pixels + 12);

        // Load `bits` into simd 64-bit vector register
        uint16x4_t mask_0_u16x4 = vcreate_u16(bits_0);
        uint16x4_t mask_1_u16x4 = vcreate_u16(bits_1);
        uint16x4_t mask_2_u16x4 = vcreate_u16(bits_2);
        uint16x4_t mask_3_u16x4 = vcreate_u16(bits_3);
        // Expand mask to cover each pixel (uint32_t)
        uint32x4_t mask_0_u32 = vmovl_u16(mask_0_u16x4);
        uint32x4_t mask_1_u32 = vmovl_u16(mask_1_u16x4);
        uint32x4_t mask_2_u32 = vmovl_u16(mask_2_u16x4);
        uint32x4_t mask_3_u32 = vmovl_u16(mask_3_u16x4);
        // Expand mask to be all-1 or all-0 mask for each pixel
        mask_0_u32 = vcgtq_u32(mask_0_u32, m_zeros);
        mask_1_u32 = vcgtq_u32(mask_1_u32, m_zeros);
        mask_2_u32 = vcgtq_u32(mask_2_u32, m_zeros);
        mask_3_u32 = vcgtq_u32(mask_3_u32, m_zeros);

        uint32x4_t out_0_u32, out_1_u32, out_2_u32, out_3_u32;
        if (m_replace_if_zero){
            // bit select intrinsic: 
            // vbslq_u32(a, b, c), for 1 bits in a, choose b; for 0 bits in a, choose c
            out_0_u32 = vbslq_u32(mask_0_u32, pixels_0_u32, m_replacement_u32);
            out_1_u32 = vbslq_u32(mask_1_u32, pixels_1_u32, m_replacement_u32);
            out_2_u32 = vbslq_u32(mask_2_u32, pixels_2_u32, m_replacement_u32);
            out_3_u32 = vbslq_u32(mask_3_u32, pixels_3_u32, m_replacement_u32);
        }else{
            out_0_u32 = vbslq_u32(mask_0_u32, m_replacement_u32, pixels_0_u32);
            out_1_u32 = vbslq_u32(mask_1_u32, m_replacement_u32, pixels_1_u32);
            out_2_u32 = vbslq_u32(mask_2_u32, m_replacement_u32, pixels_2_u32);
            out_3_u32 = vbslq_u32(mask_3_u32, m_replacement_u32, pixels_3_u32);
        }
        vst1q_u32(pixels, out_0_u32);
        vst1q_u32(pixels + 4, out_1_u32);
        vst1q_u32(pixels + 8, out_2_u32);
        vst1q_u32(pixels + 12, out_3_u32);
    }

private:
    const uint32x4_t m_replacement_u32;
    const bool m_replace_if_zero;
    const uint32x4_t m_zeros;
};

// Compress given pixels buffer (of up to 64-pixel long) into bit map and store in one uint64_t.
class Compressor_RgbRange_arm64_NEON{
public:
    Compressor_RgbRange_arm64_NEON(uint32_t mins, uint32_t maxs)
        : m_mins_u8(vreinterpretq_u8_u32(vdupq_n_u32(mins)))
        , m_maxs_u8(vreinterpretq_u8_u32(vdupq_n_u32(maxs)))
        , m_zeros(vreinterpretq_u32_u8(vdupq_n_u8(0)))
    {}

    // Convert a row of 64 pixels to bit map fit into uint64_t
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        for(size_t c = 0; c < 64; c += 16){
            bits |= convert16(pixels + c) << c;
        }
        return bits;
    }
    // Convert a row of `count` pixels to bit map fit into uint64_t
    // count <= 64
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;

        for(size_t i = 0; i < count / 16; i++){
            bits |= convert16(pixels + c) << c;
            c += 16;
        }

        count %= 16;
        for(size_t i = 0; i < count / 4; i++, c+=4){
            const uint8x16_t pixel = vld1q_u8((const uint8_t*)(pixels + c));
            bits |= convert4(pixel) << c;
        }
        count %= 4;
        if (count){
            PartialWordAccess_arm64_NEON loader(count * sizeof(uint32_t));
            const uint8x16_t pixel = loader.load(pixels + c);
            const uint64_t mask = ((uint64_t)1 << count) - 1;
            bits |= (convert4(pixel) & mask) << c;
        }
        return bits;
    }

private:
    // Convert four pixels to four 0/1 bits according to RGB color range
    // Return a uint64_t where the lowest four bits contain the converted bits for each pixel.
    PA_FORCE_INLINE uint64_t convert4(const uint8x16_t& pixel) const{
        // Check if mins > pixel per color channel
        uint8x16_t cmp0 = vcgtq_u8(m_mins_u8, pixel);
        // Check if pixel > maxs per color channel
        uint8x16_t cmp1 = vcgtq_u8(pixel, m_maxs_u8);
        // cmp: if mins > pixel or pixel > maxs per color channel
        uint8x16_t cmp = vorrq_u8(cmp0, cmp1);
        // cmp_32x4: if each pixel is within the range
        // If a pixel is within range, its uint32_t in `cmp_32x4` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_32x4 = vceqq_u32(vreinterpretq_u32_u8(cmp), m_zeros);
        return (cmp_32x4[0] & 0x1) | (cmp_32x4[1] & 0x2) | (cmp_32x4[2] & 0x4) | (cmp_32x4[3] & 0x8);
    }

    // Convert 16 pixels to 16 0/1 bits according to RGB color range
    // Return a uint64_t where the lowest bits contain the converted bits for each pixel.
    PA_FORCE_INLINE uint64_t convert16(const uint32_t* pixels) const{
        uint8x16x4_t pixelx4 = vld1q_u8_x4((const uint8_t*)pixels);

        // cmpx_min: Check if mins > pixel per color channel
        // cmpx_max: Check if pixel > maxs per color channel
        uint8x16_t cmp0_min = vcgtq_u8(m_mins_u8, pixelx4.val[0]);
        uint8x16_t cmp0_max = vcgtq_u8(pixelx4.val[0], m_maxs_u8);
        uint8x16_t cmp1_min = vcgtq_u8(m_mins_u8, pixelx4.val[1]);
        uint8x16_t cmp1_max = vcgtq_u8(pixelx4.val[1], m_maxs_u8);
        uint8x16_t cmp2_min = vcgtq_u8(m_mins_u8, pixelx4.val[2]);
        uint8x16_t cmp2_max = vcgtq_u8(pixelx4.val[2], m_maxs_u8);
        uint8x16_t cmp3_min = vcgtq_u8(m_mins_u8, pixelx4.val[3]);
        uint8x16_t cmp3_max = vcgtq_u8(pixelx4.val[3], m_maxs_u8);

        // cmp: if mins > pixel or pixel > maxs per color channel
        uint8x16_t cmp0 = vorrq_u8(cmp0_min, cmp0_max);
        uint8x16_t cmp1 = vorrq_u8(cmp1_min, cmp1_max);
        uint8x16_t cmp2 = vorrq_u8(cmp2_min, cmp2_max);
        uint8x16_t cmp3 = vorrq_u8(cmp3_min, cmp3_max);

        // cmp_32x4: if each pixel is within the range
        // If a pixel is within range, its uint32_t in `cmp_32x4` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp0_32x4 = vceqq_u32(vreinterpretq_u32_u8(cmp0), m_zeros);
        uint32x4_t cmp1_32x4 = vceqq_u32(vreinterpretq_u32_u8(cmp1), m_zeros);
        uint32x4_t cmp2_32x4 = vceqq_u32(vreinterpretq_u32_u8(cmp2), m_zeros);
        uint32x4_t cmp3_32x4 = vceqq_u32(vreinterpretq_u32_u8(cmp3), m_zeros);

        // Combine pixel bits together
        return (cmp0_32x4[0] & 0x01) | (cmp0_32x4[1] & 0x02) | (cmp0_32x4[2] & 0x04) | (cmp0_32x4[3] & 0x08)
             | (cmp1_32x4[0] & 0x10) | (cmp1_32x4[1] & 0x20) | (cmp1_32x4[2] & 0x40) | (cmp1_32x4[3] & 0x80)
             | (cmp2_32x4[0] & 0x100) | (cmp2_32x4[1] & 0x200) | (cmp2_32x4[2] & 0x400) | (cmp2_32x4[3] & 0x800)
             | (cmp3_32x4[0] & 0x1000) | (cmp3_32x4[1] & 0x2000) | (cmp3_32x4[2] & 0x4000) | (cmp3_32x4[3] & 0x8000);
    }

private:
    uint8x16_t m_mins_u8;
    uint8x16_t m_maxs_u8;
    const uint32x4_t m_zeros;
};


class Compressor_RgbEuclidean_arm64_NEON{
public:
    Compressor_RgbEuclidean_arm64_NEON(uint32_t expected_color, double max_euclidean_distance)
        : m_expected_color_rgb_u8(vreinterpretq_u8_u32(vdupq_n_u32(expected_color & 0x00ffffff)))
        , m_distance_squared_u32(vdupq_n_u32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels) const{
        uint64_t bits = 0;
        for(size_t c = 0; c < 64; c += 4){
            bits |= convert4(pixels + c) << c;
        }
        return bits;
    }
    PA_FORCE_INLINE uint64_t convert64(const uint32_t* pixels, size_t count) const{
        uint64_t bits = 0;
        size_t c = 0;

        for(size_t i = 0; i < count / 4; i++, c+=4){
            bits |= convert4(pixels + c) << c;
        }
        count %= 4;
        if (count){
            PartialWordAccess_arm64_NEON loader(count * sizeof(uint32_t));
            const uint8x16_t pixel = loader.load(pixels + c);
            const uint64_t mask = ((uint64_t)1 << count) - 1;
            bits |= (convert4(pixel) & mask) << c;
        }
        return bits;
    }

private:
    PA_FORCE_INLINE uint64_t convert4(const uint32_t* pixel) const{
        uint32x4_t in_u32 = vld1q_u32(pixel);
        return convert4(vreinterpretq_u8_u32(in_u32));
    }
    PA_FORCE_INLINE uint64_t convert4(const uint32x4_t& in_u32) const{
        // subtract the expected values
        uint32x4_t in_dif_u32 = vreinterpretq_u32_u8(vabdq_u8(vreinterpretq_u8_u32(in_u32), m_expected_color_rgb_u8));

        // Get green channel
        uint32x4_t in_g_u32 = vandq_u32(in_dif_u32, vdupq_n_u32(0x0000ff00));
        // Move green channel to the lower end of the 16-bit regions
        uint16x8_t in_g_u16 = vshrq_n_u16(vreinterpretq_u16_u32(in_g_u32), 8);
        // in_rb_u16 contains the red and blue channels. Each channel occupies a 16-bit region
        uint16x8_t in_rb_u16 = vandq_u16(vreinterpretq_u16_u32(in_dif_u32), vdupq_n_u16(0x00ff));

        // Square operation
        uint16x8_t in_g2_u16 = vmulq_u16(in_g_u16, in_g_u16);
        uint16x8_t in_r2b2_u16 = vmulq_u16(in_rb_u16, in_rb_u16);

        uint32x4_t in_g2_u32 = vreinterpretq_u32_u16(in_g2_u16);
        // Use pairwise addition and accumulate to add r2, g2, and b2 together
        uint32x4_t sum_sqr_u32 = vpadalq_u16(in_g2_u32, in_r2b2_u16);

        // cmp_u32: if each pixel is within range (sum_sqr <= max_distance_squared), its uint32_t in `cmp_u32` is all 1 bits,
        // otherwise, all 0 bits
        uint32x4_t cmp_u32 = vcleq_u32(sum_sqr_u32, m_distance_squared_u32);
        return (cmp_u32[0] & 0x01) | (cmp_u32[1] & 0x02) | (cmp_u32[2] & 0x04) | (cmp_u32[3] & 0x08);
    }

private:
    uint8x16_t m_expected_color_rgb_u8;
    uint32x4_t m_distance_squared_u32;
};



}
}
#endif
