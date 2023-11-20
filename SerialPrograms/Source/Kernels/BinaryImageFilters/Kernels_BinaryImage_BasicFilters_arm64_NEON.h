/*  Binary Image Basic Filters (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_BinaryImage_BasicFilters_arm64_NEON_H
#define PokemonAutomation_Kernels_BinaryImage_BasicFilters_arm64_NEON_H

#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"

namespace PokemonAutomation{
namespace Kernels{

// Change color of an array of pixels based on values from a bitmap that corresponds to the pixel array.
// If `replace_if_zero` is true, change color of pixels that correspond to 0 bits.
// Otherwise, chagne color of pixels that correspond to 1 bits.
class FilterByMask_arm64_NEON{
public:
    FilterByMask_arm64_NEON(uint32_t replacement, bool replace_if_zero)
        : m_replacement_pixel(replacement)
        , m_replacement(vdupq_n_u32(replacement))
        , m_replace_if_zero_bool(replace_if_zero)
        , m_replace_if_zero(vreinterpretq_u32_u8(vdupq_n_u8(replace_if_zero ? 0xff: 0)))
        , m_zeros(vreinterpretq_u32_u8(vdupq_n_u8(0)))
        , m_lasts(vdupq_n_u32(1))
    {}

    // Given 64 bits stored in `uint64_t`, use it to set colors to 64 pixels in `pixels`.
    // If filter constructor parameter `replace_if_zero` is true, the pixels corresponding to
    // 0-bits are set to color `replacement` (another filter constructor parameter).
    // Otherwise, pixels corresponding to 1-bits are set to the color.
    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels) const{
        for(int i = 0; i < 64; i+=8){
            filter8((bits >> i) & 0xFF, pixels + i);
        }
    }

    // partial version of filter64(bits, pixels)
    PA_FORCE_INLINE void filter64(uint64_t bits, uint32_t* pixels, size_t count) const{
        size_t lc = count / 4;
        while (lc--){
            uint32x4_t pixel = vld1q_u32(pixels);
            // Change color in the four pixels according to the lowest four bits in `bits`
            uint32x4_t filtered_pixel = filter4(bits & 0xF, pixel);

            vst1q_u32(pixels, filtered_pixel);
            pixels += 4;
            bits >>= 4;
        }
        count %= 4;
        if (count){
            // Load a partial of 4-pixel long data
            PartialWordAccess_arm64_NEON loader(count * sizeof(uint32_t));
            uint32x4_t pixel = vreinterpretq_u32_u8(loader.load(pixels));
            // Change color in the pixels according to the lowest bits in `bits`
            pixel = filter4(bits & 0xF, pixel);
            switch (count){
                case 1:
                    pixels[0] = vgetq_lane_u32(pixel, 0);
                    break;
                case 2:
                    pixels[0] = vgetq_lane_u32(pixel, 0);
                    pixels[1] = vgetq_lane_u32(pixel, 1);
                    break;
                default:
                    pixels[0] = vgetq_lane_u32(pixel, 0);
                    pixels[1] = vgetq_lane_u32(pixel, 1);
                    pixels[2] = vgetq_lane_u32(pixel, 2);
            }
        }
    }

private:

    // Change color in the 8 pixels according to the lowest 8 bits in `bits`
    PA_FORCE_INLINE void filter8(uint32_t bits64, uint32_t* pixels) const{
        const uint32_t uint32_max = 0xFFFFFFFF;
        if (m_replace_if_zero_bool){
            uint32_t mask0 = (bits64 & 0x1) - 1;
            uint32_t mask1 = ((bits64 >> 1) & 0x1) - 1;
            uint32_t mask2 = ((bits64 >> 2) & 0x1) - 1;
            uint32_t mask3 = ((bits64 >> 3) & 0x1) - 1;
            uint32_t mask4 = ((bits64 >> 4) & 0x1) - 1;
            uint32_t mask5 = ((bits64 >> 5) & 0x1) - 1;
            uint32_t mask6 = ((bits64 >> 6) & 0x1) - 1;
            uint32_t mask7 = ((bits64 >> 7) & 0x1) - 1;

            pixels[0] = (mask0 & m_replacement_pixel) | (pixels[0] & (uint32_max - mask0));
            pixels[1] = (mask1 & m_replacement_pixel) | (pixels[1] & (uint32_max - mask1));
            pixels[2] = (mask2 & m_replacement_pixel) | (pixels[2] & (uint32_max - mask2));
            pixels[3] = (mask3 & m_replacement_pixel) | (pixels[3] & (uint32_max - mask3));
            pixels[4] = (mask4 & m_replacement_pixel) | (pixels[4] & (uint32_max - mask4));
            pixels[5] = (mask5 & m_replacement_pixel) | (pixels[5] & (uint32_max - mask5));
            pixels[6] = (mask6 & m_replacement_pixel) | (pixels[6] & (uint32_max - mask6));
            pixels[7] = (mask7 & m_replacement_pixel) | (pixels[7] & (uint32_max - mask7));
        } else{
            uint32_t mask0 = (bits64 & 0x1) - 1;
            uint32_t mask1 = ((bits64 >> 1) & 0x1) - 1;
            uint32_t mask2 = ((bits64 >> 2) & 0x1) - 1;
            uint32_t mask3 = ((bits64 >> 3) & 0x1) - 1;
            uint32_t mask4 = ((bits64 >> 4) & 0x1) - 1;
            uint32_t mask5 = ((bits64 >> 5) & 0x1) - 1;
            uint32_t mask6 = ((bits64 >> 6) & 0x1) - 1;
            uint32_t mask7 = ((bits64 >> 7) & 0x1) - 1;

            pixels[0] = (mask0 & pixels[0]) | (m_replacement_pixel & (uint32_max - mask0));
            pixels[1] = (mask1 & pixels[1]) | (m_replacement_pixel & (uint32_max - mask1));
            pixels[2] = (mask2 & pixels[2]) | (m_replacement_pixel & (uint32_max - mask2));
            pixels[3] = (mask3 & pixels[3]) | (m_replacement_pixel & (uint32_max - mask3));
            pixels[4] = (mask4 & pixels[4]) | (m_replacement_pixel & (uint32_max - mask4));
            pixels[5] = (mask5 & pixels[5]) | (m_replacement_pixel & (uint32_max - mask5));
            pixels[6] = (mask6 & pixels[6]) | (m_replacement_pixel & (uint32_max - mask6));
            pixels[7] = (mask7 & pixels[7]) | (m_replacement_pixel & (uint32_max - mask7));
        }
    }

    // Change color in the 16 pixels according to the lowest 16 bits in `bits`
    PA_FORCE_INLINE void filter16_NEON(uint32_t bits64, uint32_t* pixels) const{
        // const uint32_t uint32_max = 0xFFFFFFFF;
        if (m_replace_if_zero_bool){

            uint32x4x4_t pixel_32x4x4 = vld1q_u32_x4(pixels);

            uint32_t bit_rs_00 = (bits64 >> 0);
            uint32_t bit_rs_01 = (bits64 >> 1);
            uint32_t bit_rs_02 = (bits64 >> 2);
            uint32_t bit_rs_03 = (bits64 >> 3);
            uint32x4_t mask0 = vtstq_u32(uint32x4_t{bit_rs_00, bit_rs_01, bit_rs_02, bit_rs_03}, m_lasts);
            uint32_t bit_rs_04 = (bits64 >> 4);
            uint32_t bit_rs_05 = (bits64 >> 5);
            uint32_t bit_rs_06 = (bits64 >> 6);
            uint32_t bit_rs_07 = (bits64 >> 7);
            uint32x4_t mask1 = vtstq_u32(uint32x4_t{bit_rs_04, bit_rs_05, bit_rs_06, bit_rs_07}, m_lasts);
            uint32_t bit_rs_08 = (bits64 >> 8);
            uint32_t bit_rs_09 = (bits64 >> 9);
            uint32_t bit_rs_10 = (bits64 >> 10);
            uint32_t bit_rs_11 = (bits64 >> 11);
            uint32x4_t mask2 = vtstq_u32(uint32x4_t{bit_rs_08, bit_rs_09, bit_rs_10, bit_rs_11}, m_lasts);
            uint32_t bit_rs_12 = (bits64 >> 12);
            uint32_t bit_rs_13 = (bits64 >> 13);
            uint32_t bit_rs_14 = (bits64 >> 14);
            uint32_t bit_rs_15 = (bits64 >> 15);
            uint32x4_t mask3 = vtstq_u32(uint32x4_t{bit_rs_12, bit_rs_13, bit_rs_14, bit_rs_15}, m_lasts);

            uint32x4x4_t filtered_pixel_32x4x4;
            
            filtered_pixel_32x4x4.val[0] = ((m_replace_if_zero - mask0) & m_replacement) + ((mask0) & pixel_32x4x4.val[0]);
            filtered_pixel_32x4x4.val[1] = ((m_replace_if_zero - mask1) & m_replacement) + ((mask1) & pixel_32x4x4.val[1]);
            filtered_pixel_32x4x4.val[2] = ((m_replace_if_zero - mask2) & m_replacement) + ((mask2) & pixel_32x4x4.val[2]);
            filtered_pixel_32x4x4.val[3] = ((m_replace_if_zero - mask3) & m_replacement) + ((mask3) & pixel_32x4x4.val[3]);

            // vbslq_u32(a, b, c), for 1 bits in a, choose b; for 0 bits in a, choose c
            // filtered_pixel_32x4x4.val[0] = vbslq_u32(vsubq_u32(mask0, m_lasts), m_replacement, pixel_32x4x4.val[0]);
            // filtered_pixel_32x4x4.val[1] = vbslq_u32(vsubq_u32(mask1, m_lasts), m_replacement, pixel_32x4x4.val[1]);
            // filtered_pixel_32x4x4.val[2] = vbslq_u32(vsubq_u32(mask2, m_lasts), m_replacement, pixel_32x4x4.val[2]);
            // filtered_pixel_32x4x4.val[3] = vbslq_u32(vsubq_u32(mask3, m_lasts), m_replacement, pixel_32x4x4.val[3]);     

            vst1q_u32_x4(pixels, filtered_pixel_32x4x4);
        } else{
            // TODO:
        }
    }

    // Change color in the four pixels according to the lowest four bits in `bits`
    PA_FORCE_INLINE void filter4(uint32_t bits64, uint32_t* pixels) const{
        // uint32x4_t pixel = vld1q_u32(pixels);
        if (m_replace_if_zero_bool){
            uint32_t check0 = (bits64 & 0x1);
            pixels[0] = (1 - check0) * m_replacement_pixel + pixels[0] * check0;
            uint32_t check1 = ((bits64 >> 1) & 0x1);
            pixels[1] = (1 - check1) * m_replacement_pixel + pixels[1] * check1;
            uint32_t check2 = ((bits64 >> 2) & 0x1);
            pixels[2] = (1 - check2) * m_replacement_pixel + pixels[2] * check2;
            uint32_t check3 = ((bits64 >> 3) & 0x1);
            pixels[3] = (1 - check3) * m_replacement_pixel + pixels[3] * check3;
        }
        else{
            if ((bits64 & 0x1) != 0){
                pixels[0] = m_replacement_pixel;
            }
            if ((bits64 & 0x2) != 0){
                pixels[1] = m_replacement_pixel;
            }
            if ((bits64 & 0x4) != 0){
                pixels[2] = m_replacement_pixel;
            }
            if ((bits64 & 0x8) != 0){
                pixels[3] = m_replacement_pixel;
            }
        }
    }

    // Change color in the four pixels according to the lowest four bits in `bits`
    PA_FORCE_INLINE uint32x4_t filter4(uint64_t bits64, uint32x4_t pixel) const{
        // Duplicate 4-bit pattern into four uint16_t places in `bits`
        bits64 *= 0x0001000100010001;
        // convert each uint16_t to be one bit from the lowest four bits in input `bits`
        bits64 &= 0x0008000400020001;

        // Load `bits` into simd 64-bit vector register
        uint16x4_t mask_u16x4 = vcreate_u16(bits64);
        // Expand mask to cover each pixel (uint32_t)
        uint32x4_t mask = vmovl_u16(mask_u16x4);
        // Expand mask to be all-1 or all-0 mask for each pixel
        mask = vcgtq_u32(mask, m_zeros);
        // If m_replace_if_zero is 0xFF...FF, invert the mask
        mask = veorq_u32(mask, m_replace_if_zero);
        // bit select intrinsic: 
        // vbslq_u32(a, b, c), for 1 bits in a, choose b; for 0 bits in a, choose c
        return vbslq_u32(mask, m_replacement, pixel);
    }

private:
    const uint32_t m_replacement_pixel;
    const uint32x4_t m_replacement;
    const bool m_replace_if_zero_bool;
    const uint32x4_t m_replace_if_zero;
    const uint32x4_t m_zeros;
    const uint32x4_t m_lasts;
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
        for(; c < count; c += 4){
            const uint8x16_t pixel = vld1q_u8((const uint8_t*)(pixels + c));
            bits |= convert4(pixel) << c;
        }
        count %= 4;
        if (count){
            c -= 4;
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


}
}
#endif
