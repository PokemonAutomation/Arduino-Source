/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include "Common/Compiler.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels/Kernels_arm64_NEON.h"

namespace PokemonAutomation{
namespace Kernels{




template <typename PixelTester>
class FilterImage_Rgb32_ARM64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    FilterImage_Rgb32_ARM64_NEON(
        const PixelTester& tester,
        uint32_t replacement_color, bool replace_color_within_range
    )
        : m_tester(tester)
        , m_replacement_color_u32(vdupq_n_u32(replacement_color))
        , m_replace_color_within_range(replace_color_within_range)
        , m_count_u32(vdupq_n_u32(0))
    {}

    PA_FORCE_INLINE size_t count() const{
        // long pairwise add
        uint64x2_t sum_u64 = vpaddlq_u32(m_count_u32);
        return sum_u64[0] + sum_u64[1];
    }

    // Given 4 pixels from in[4], apply color range comparison and count the pixels that are in range.
    // The counts are stored in m_count_u32.
    // If a per-pixel mask, cmp_mask_u32 is not nullptr, it only counts the pixels covered by the mask.
    // It also changes pixels in or out of the range to have the new color m_replacement_color_u32.
    // The resulting pixels are saved in out[4]
    PA_FORCE_INLINE void process_full(uint32_t out[4], const uint32_t in[4], const uint32x4_t* cmp_mask_u32 = nullptr){
        uint32x4_t pixel = vld1q_u32(in);
        // If a pixel is within [mins, maxs], its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_u32 = m_tester.test_word(pixel);
        if (cmp_mask_u32) {
            cmp_u32 = vandq_u32(cmp_u32, *cmp_mask_u32);
        }
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by substracting 0xFFFFFFFF
        m_count_u32 = vsubq_u32(m_count_u32, cmp_u32);
        // select replacement color or in_u8 based on cmp_u32:
        uint32x4_t out_u32;
        if (m_replace_color_within_range){
            // vbslq_u32(a, b, c) for 1 bits in a, choose b; for 0 bits in a, choose c
            out_u32 = vbslq_u32(cmp_u32, m_replacement_color_u32, pixel);
        }else{
            out_u32 = vbslq_u32(cmp_u32, pixel, m_replacement_color_u32);
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
    const PixelTester m_tester;
    uint32x4_t m_replacement_color_u32;
    bool m_replace_color_within_range;
    uint32x4_t m_count_u32;
};







template <typename PixelTester>
class ToBlackWhite_Rgb32_ARM64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    ToBlackWhite_Rgb32_ARM64_NEON(
        const PixelTester& tester,
        bool in_range_black
    )
        : m_tester(tester)
        , m_in_range_color_u32(vdupq_n_u32(in_range_black ? 0xFF000000 : 0xFFFFFFFF))
        , m_out_of_range_color_u32(vdupq_n_u32(in_range_black ? 0xFFFFFFFF : 0xFF000000))
        , m_count_u32(vdupq_n_u32(0))
    {}

    PA_FORCE_INLINE size_t count() const{
        uint64x2_t sum_u64 = vpaddlq_u32(m_count_u32);
        return sum_u64[0] + sum_u64[1];
    }

    // Given 4 pixels from in[4], apply color range comparison and count the pixels that are in range.
    // The counts are stored in m_count_u32.
    // If a per-pixel mask, cmp_mask_u32 is not nullptr, it only counts the pixels covered by the mask.
    // It also changes pixels into black or white depending on whether they are in range.
    // The resulting pixels are saved in out[4]
    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in, const uint32x4_t* cmp_mask_u32 = nullptr){
        uint32x4_t pixel = vld1q_u32(in);
        // If a pixel is within [mins, maxs], its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        uint32x4_t cmp_u32 = m_tester.test_word(pixel);
        if (cmp_mask_u32) {
            cmp_u32 = vandq_u32(cmp_u32, *cmp_mask_u32);
        }
        // Increase count for each pixel in range. Each uint32 lane is counted separately.
        // We achieve +=1 by substracting 0xFFFFFFFF
        m_count_u32 = vsubq_u32(m_count_u32, cmp_u32);
        // select replacement color or in_u8 based on cmp_u32:
        uint32x4_t out_u32;
        // vbslq_u32(a, b, c) for 1 bits in a, choose b; for 0 bits in a, choose c
        out_u32 = vbslq_u32(cmp_u32, m_in_range_color_u32, m_out_of_range_color_u32);

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
    const PixelTester m_tester;
    uint32x4_t m_in_range_color_u32;
    uint32x4_t m_out_of_range_color_u32;
    uint32x4_t m_count_u32;
};





}
}
