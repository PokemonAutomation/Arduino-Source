/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_ARM64_NEON.h"
#include "Kernels_ImageFilter_RGB32_Range_Routines.h"
#include "Kernels_ImageFilter_RGB32_Range.h"



namespace PokemonAutomation{
namespace Kernels{




class PixelTest_Rgb32Range_ARM64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = size_t;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Range_ARM64_NEON(
        uint32_t mins, uint32_t maxs
    )
        : m_mins_u8(vreinterpretq_u8_u32(vdupq_n_u32(mins)))
        , m_maxs_u8(vreinterpretq_u8_u32(vdupq_n_u32(maxs)))
        , m_zeros_u8(vreinterpretq_u32_u8(vdupq_n_u8(0)))
    {}
    PA_FORCE_INLINE PixelTest_Rgb32Range_ARM64_NEON(
        const ToBlackWhiteRgb32RangeFilter& filter
    )
        : m_mins_u8(vreinterpretq_u8_u32(vdupq_n_u32(filter.mins)))
        , m_maxs_u8(vreinterpretq_u8_u32(vdupq_n_u32(filter.maxs)))
        , m_zeros_u8(vreinterpretq_u32_u8(vdupq_n_u8(0)))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE uint32x4_t test_word(uint32x4_t& pixel) const{
        uint8x16_t in_u8 = vreinterpretq_u8_u32(pixel);

        // Check if mins > pixel per color channel
        uint8x16_t cmp0 = vcgtq_u8(m_mins_u8, in_u8);
        // Check if pixel > maxs per color channel
        uint8x16_t cmp1 = vcgtq_u8(in_u8, m_maxs_u8);
        // cmp: if mins > pixel or pixel > maxs per color channel
        uint8x16_t cmp_u8 = vorrq_u8(cmp0, cmp1);
        // vceqq_u32: compare bitwise equal
        // cmp_u32: if each pixel is within the range
        // If a pixel is within [mins, maxs], its uint32_t in `cmp_u32` is all 1 bits, otherwise, all 0 bits
        return vceqq_u32(vreinterpretq_u32_u8(cmp_u8), vreinterpretq_u32_u8(m_zeros_u8));
    }

private:
    uint8x16_t m_mins_u8;
    uint8x16_t m_maxs_u8;
    uint8x16_t m_zeros_u8;
};





size_t filter_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_ARM64_NEON tester(mins, maxs);
    FilterImage_Rgb32_ARM64_NEON<PixelTest_Rgb32Range_ARM64_NEON> filter(
        tester,
        replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_ARM64_NEON tester(mins, maxs);
    ToBlackWhite_Rgb32_ARM64_NEON<PixelTest_Rgb32Range_ARM64_NEON> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}






}
}
#endif
