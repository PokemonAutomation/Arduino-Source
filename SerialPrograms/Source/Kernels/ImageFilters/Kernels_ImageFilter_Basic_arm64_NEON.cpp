/*  Image Filters Basic (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <stdint.h>
#include <cstddef>
#include "Kernels/Kernels_arm64_NEON.h"
// #include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

#include <iostream>
#include <iomanip>

namespace PokemonAutomation{
namespace Kernels{

class ImageFilter_RgbRange_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    ImageFilter_RgbRange_arm64_NEON(uint32_t mins, uint32_t maxs, uint32_t replacement_color, 
        bool replace_color_within_range)
        : m_mins_u8(vreinterpretq_u8_u32(vdupq_n_u32(mins)))
        , m_maxs_u8(vreinterpretq_u8_u32(vdupq_n_u32(maxs)))
        , m_zeros_u8(vreinterpretq_u32_u8(vdupq_n_u8(0)))
        , m_replacement_color_u32(vdupq_n_u32(replacement_color))
        , m_replace_color_within_range(replace_color_within_range)
        , m_count_u32(vdupq_n_u32(0))
    {}

    PA_FORCE_INLINE size_t count() const{
        uint64x2_t sum_u64 = vpaddlq_u32(m_count_u32);
        return sum_u64[0] + sum_u64[1];
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint8x16_t in_u8 = vreinterpretq_u8_u32(vld1q_u32(in));

        // Check if mins > pixel per color channel
        uint8x16_t cmp0 = vcgtq_u8(m_mins_u8, in_u8);
        // Check if pixel > maxs per color channel
        uint8x16_t cmp1 = vcgtq_u8(in_u8, m_maxs_u8);
        // cmp: if mins > pixel or pixel > maxs per color channel
        uint8x16_t cmp_u8 = vorrq_u8(cmp0, cmp1);
        // cmp_u32: if each pixel is within the range
        // If a pixel is within [mins, maxs], its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_u32 = vceqq_u32(vreinterpretq_u32_u8(cmp_u8), m_zeros_u8);
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by substracting 0xFFFFFFFF
        m_count_u32 = vsubq_u32(m_count_u32, cmp_u32);
        // select replacement color or in_u8 based on cmp_u32:
        uint32x4_t out_u32;
        if (m_replace_color_within_range){
            // vbslq_u32(a, b, c) for 1 bits in a, choose b; for 0 bits in a, choose c
            out_u32 = vbslq_u32(cmp_u32, m_replacement_color_u32, vreinterpretq_u32_u8(in_u8));
        }else{
            out_u32 = vbslq_u32(cmp_u32, vreinterpretq_u32_u8(in_u8), m_replacement_color_u32);
        }
        vst1q_u32(out, out_u32);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        uint32_t buffer_in[4], buffer_out[4];
        memcpy(buffer_in, in, sizeof(uint32_t) * left);
        process_full(buffer_out, buffer_in);
        memcpy(out, buffer_out, sizeof(uint32_t) * left);
    }

private:
    uint8x16_t m_mins_u8;
    uint8x16_t m_maxs_u8;
    uint8x16_t m_zeros_u8;
    uint32x4_t m_replacement_color_u32;
    bool m_replace_color_within_range;
    uint32x4_t m_count_u32;
};



size_t filter_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement_color, bool replace_color_within_range
){
    ImageFilter_RgbRange_arm64_NEON filter(mins, maxs, replacement_color, replace_color_within_range);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilter_RgbRange_arm64_NEON>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





class ImageFilter_RgbEuclidean_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    ImageFilter_RgbEuclidean_arm64_NEON(uint32_t expected_color, double max_euclidean_distance,
        uint32_t replacement_color, bool replace_color_within_range)
        : m_expected_color_rgb_u8(vreinterpretq_u8_u32(vdupq_n_u32(expected_color & 0x00ffffff)))
        , m_distance_squared_u32(vdupq_n_u32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
        , m_replacement_color_u32(vdupq_n_u32(replacement_color))
        , m_replace_color_within_range(replace_color_within_range)
        , m_count_u32(vdupq_n_u32(0))
    {}

    PA_FORCE_INLINE size_t count() const{
        uint64x2_t sum_u64 = vpaddlq_u32(m_count_u32);
        return sum_u64[0] + sum_u64[1];
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32x4_t in_u32 = vld1q_u32(in);
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

        // cmp_u32: if each pixel is within the range, its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_u32 = vcleq_u32(sum_sqr_u32, m_distance_squared_u32);
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by substracting 0xFFFFFFFF
        m_count_u32 = vsubq_u32(m_count_u32, cmp_u32);
        // select replacement color or in_u8 based on cmp_u32:
        uint32x4_t out_u32;
        if (m_replace_color_within_range){
            // vbslq_u32(a, b, c) for 1 bits in a, choose b; for 0 bits in a, choose c
            out_u32 = vbslq_u32(cmp_u32, m_replacement_color_u32, in_u32);
        }else{
            out_u32 = vbslq_u32(cmp_u32, in_u32, m_replacement_color_u32);
        }
        vst1q_u32(out, out_u32);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        uint32_t buffer_in[4], buffer_out[4];
        memcpy(buffer_in, in, sizeof(uint32_t) * left);
        process_full(buffer_out, buffer_in);
        memcpy(out, buffer_out, sizeof(uint32_t) * left);
    }

private:
    uint8x16_t m_expected_color_rgb_u8;
    uint32x4_t m_distance_squared_u32;
    uint32x4_t m_replacement_color_u32;
    bool m_replace_color_within_range;
    uint32x4_t m_count_u32;

private:

};
size_t filter_rgb32_euclidean_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement_color, bool replace_color_within_range
){
    ImageFilter_RgbEuclidean_arm64_NEON filter(expected, max_euclidean_distance, replacement_color, replace_color_within_range);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}








class ToBlackWhite_RgbRange_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    ToBlackWhite_RgbRange_arm64_NEON(uint32_t mins, uint32_t maxs, bool in_range_black)
        : m_mins_u8(vreinterpretq_u8_u32(vdupq_n_u32(mins)))
        , m_maxs_u8(vreinterpretq_u8_u32(vdupq_n_u32(maxs)))
        , m_zeros_u8(vreinterpretq_u32_u8(vdupq_n_u8(0)))
        , m_in_range_color_u32(vdupq_n_u32(in_range_black ? 0xFF000000 : 0xFFFFFFFF))
        , m_out_of_range_color_u32(vdupq_n_u32(in_range_black ? 0xFFFFFFFF : 0xFF000000))
        , m_count_u32(vdupq_n_u32(0))
    {}

    PA_FORCE_INLINE size_t count() const{
        uint64x2_t sum_u64 = vpaddlq_u32(m_count_u32);
        return sum_u64[0] + sum_u64[1];
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint8x16_t in_u8 = vreinterpretq_u8_u32(vld1q_u32(in));

        // Check if mins > pixel per color channel
        uint8x16_t cmp0 = vcgtq_u8(m_mins_u8, in_u8);
        // Check if pixel > maxs per color channel
        uint8x16_t cmp1 = vcgtq_u8(in_u8, m_maxs_u8);
        // cmp: if mins > pixel or pixel > maxs per color channel
        uint8x16_t cmp_u8 = vorrq_u8(cmp0, cmp1);
        // cmp_u32: if each pixel is within the range
        // If a pixel is within [mins, maxs], its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_u32 = vceqq_u32(vreinterpretq_u32_u8(cmp_u8), m_zeros_u8);
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by substracting 0xFFFFFFFF
        m_count_u32 = vsubq_u32(m_count_u32, cmp_u32);
        // select replacement color or in_u8 based on cmp_u32:
        uint32x4_t out_u32;
        // vbslq_u32(a, b, c) for 1 bits in a, choose b; for 0 bits in a, choose c
        out_u32 = vbslq_u32(cmp_u32, m_in_range_color_u32, m_out_of_range_color_u32);
        
        vst1q_u32(out, out_u32);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        uint32_t buffer_in[4], buffer_out[4];
        memcpy(buffer_in, in, sizeof(uint32_t) * left);
        process_full(buffer_out, buffer_in);
        memcpy(out, buffer_out, sizeof(uint32_t) * left);
    }

private:
    uint8x16_t m_mins_u8;
    uint8x16_t m_maxs_u8;
    uint8x16_t m_zeros_u8;
    uint32x4_t m_in_range_color_u32;
    uint32x4_t m_out_of_range_color_u32;
    uint32x4_t m_count_u32;
};



size_t to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ToBlackWhite_RgbRange_arm64_NEON filter(mins, maxs, in_range_black);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    return to_blackwhite_rbg32<ToBlackWhite_RgbRange_arm64_NEON>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}








}
}
#endif
