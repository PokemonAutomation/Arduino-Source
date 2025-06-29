/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_ARM64_NEON.h"

namespace PokemonAutomation{
namespace Kernels{



class PixelTest_Rgb32Euclidean_ARM64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Euclidean_ARM64_NEON(
        uint32_t expected_color, double max_euclidean_distance
    )
        : m_expected_color_rgb_u8(vreinterpretq_u8_u32(vdupq_n_u32(expected_color & 0x00ffffff)))
        , m_distance_squared_u32(vdupq_n_u32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE uint32x4_t test_word(uint32x4_t& pixel) const{
        uint32x4_t& in_u32 = pixel;
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
        return vcleq_u32(sum_sqr_u32, m_distance_squared_u32);
    }

private:
    uint8x16_t m_expected_color_rgb_u8;
    uint32x4_t m_distance_squared_u32;
};



size_t filter_rgb32_euclidean_ARM64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_ARM64_NEON tester(
        expected, max_euclidean_distance
    );
    FilterImage_Rgb32_ARM64_NEON<PixelTest_Rgb32Euclidean_ARM64_NEON> filter(
        tester, replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_euclidean_ARM64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t expected, double max_euclidean_distance
){
    PixelTest_Rgb32Euclidean_ARM64_NEON tester(
        expected, max_euclidean_distance
    );
    ToBlackWhite_Rgb32_ARM64_NEON<PixelTest_Rgb32Euclidean_ARM64_NEON> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}



#if 0

class ImageFilter_RgbEuclidean_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    ImageFilter_RgbEuclidean_arm64_NEON(
        uint32_t replacement_color, bool replace_color_within_range,
        uint32_t expected_color, double max_euclidean_distance
    )
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

    // Given 4 pixels from in[4], apply color range comparison and count the pixels that are in range.
    // The counts are stored in m_count_u32.
    // If a per-pixel mask, cmp_mask_u32 is not nullptr, it only counts the pixels covered by the mask.
    // It also changes pixels in or out of the range to have the new color m_replacement_color_u32.
    // The resulting pixels are saved in out[4]
    PA_FORCE_INLINE void process_full(uint32_t out[4], const uint32_t in[4], const uint32x4_t* cmp_mask_u32 = nullptr){
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
        if (cmp_mask_u32) {
            cmp_u32 = vandq_u32(cmp_u32, *cmp_mask_u32);
        }
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by subtracting 0xFFFFFFFF
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
    // Same as `process_full()` but only process `left` (< 4) pixels
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        uint32x4_t cmp_mask_u32 = vreinterpretq_u32_u8(PartialWordAccess_arm64_NEON::create_front_mask(left * 4));
        uint32_t buffer_in[4], buffer_out[4];
        memcpy(buffer_in, in, sizeof(uint32_t) * left);
        process_full(buffer_out, buffer_in, &cmp_mask_u32);
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
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    ImageFilter_RgbEuclidean_arm64_NEON filter(
        replacement, replace_color_within_range,
        expected, max_euclidean_distance
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
#endif




}
}
#endif
