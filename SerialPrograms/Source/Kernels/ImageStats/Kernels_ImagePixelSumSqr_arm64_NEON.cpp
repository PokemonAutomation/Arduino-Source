/*  Pixel Sum + Sum of Squares (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Common/Cpp/Exceptions.h"
#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels_ImagePixelSumSqr.h"

namespace PokemonAutomation{
namespace Kernels{


void pixel_sum_sqr_Default(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
);



//  Accumulate 4 pixels stored as uint32 lanes (ARGB, B in the lowest byte) into the
//  per-lane uint32 accumulators. This mirrors one iteration of the SSE4.1 kernel and is
//  used for the pixels left over after the 16-pixel main loop in `pixel_sum_sqr_arm64_NEON()`.
//  `m` is the raw alpha-buffer pixels: a pixel is active if its alpha byte is >= 128,
//  i.e. the sign bit of the uint32 is set.
PA_FORCE_INLINE void pixel_sum_sqr_arm64_NEON_4px(
    uint32x4_t& sumB, uint32x4_t& sumG, uint32x4_t& sumR, uint32x4_t& sumA,
    uint32x4_t& sqrB, uint32x4_t& sqrG, uint32x4_t& sqrR,
    uint32x4_t p, uint32x4_t m
){
    //  Arithmetic shift turns the alpha sign bit into an all-ones/all-zeros lane mask.
    m = vreinterpretq_u32_s32(vshrq_n_s32(vreinterpretq_s32_u32(m), 31));
    p = vandq_u32(p, m);

    const uint32x4_t byte_mask = vdupq_n_u32(0x000000ff);
    uint32x4_t r0 = vandq_u32(p, byte_mask);
    uint32x4_t r1 = vandq_u32(vshrq_n_u32(p, 8), byte_mask);
    uint32x4_t r2 = vandq_u32(vshrq_n_u32(p, 16), byte_mask);

    sumB = vaddq_u32(sumB, r0);
    sumG = vaddq_u32(sumG, r1);
    sumR = vaddq_u32(sumR, r2);
    //  Active lanes of `m` are 0xffffffff == -1, so subtracting counts them.
    sumA = vsubq_u32(sumA, m);

    sqrB = vmlaq_u32(sqrB, r0, r0);
    sqrG = vmlaq_u32(sqrG, r1, r1);
    sqrR = vmlaq_u32(sqrR, r2, r2);
}

//  Compute pixel sums and sums of squares of one image row of `width` pixels.
//  Requires `width >= 4` so the final partial load can safely read bytes before
//  the end of the row instead of past it.
//
//  Main loop processes 16 pixels per iteration: `vld4q_u8()` de-interleaves 64 bytes of
//  BGRA pixels into four uint8x16 vectors, one per channel. Byte sums are widened with
//  pairwise adds (u8 -> u16 -> u32) and squares are computed with widening multiplies
//  (u8 * u8 -> u16), then pairwise accumulated into uint32 lanes.
//
//  Overflow: width <= 65535, so each of the four uint32 lanes receives at most
//  ~16384 pixels, and 16384 * 255^2 < 2^32.
PA_FORCE_INLINE void pixel_sum_sqr_arm64_NEON(
    PixelSums& sums,
    uint16_t width,
    const uint32_t* image,
    const uint32_t* alpha
){
    uint32x4_t sumB = vdupq_n_u32(0);
    uint32x4_t sumG = vdupq_n_u32(0);
    uint32x4_t sumR = vdupq_n_u32(0);
    uint32x4_t sumA = vdupq_n_u32(0);
    uint32x4_t sqrB = vdupq_n_u32(0);
    uint32x4_t sqrG = vdupq_n_u32(0);
    uint32x4_t sqrR = vdupq_n_u32(0);

    size_t lc = width / 16;
    while (lc--){
        //  val[0] = B, val[1] = G, val[2] = R, val[3] = A for 16 pixels.
        uint8x16x4_t p = vld4q_u8((const uint8_t*)image);
        uint8x16_t a = vld4q_u8((const uint8_t*)alpha).val[3];

        //  0xff where alpha >= 128, else 0.
        uint8x16_t m = vreinterpretq_u8_s8(vshrq_n_s8(vreinterpretq_s8_u8(a), 7));
        uint8x16_t b = vandq_u8(p.val[0], m);
        uint8x16_t g = vandq_u8(p.val[1], m);
        uint8x16_t r = vandq_u8(p.val[2], m);

        sumB = vpadalq_u16(sumB, vpaddlq_u8(b));
        sumG = vpadalq_u16(sumG, vpaddlq_u8(g));
        sumR = vpadalq_u16(sumR, vpaddlq_u8(r));
        sumA = vpadalq_u16(sumA, vpaddlq_u8(vshrq_n_u8(a, 7)));

        sqrB = vpadalq_u16(sqrB, vmull_u8(vget_low_u8(b), vget_low_u8(b)));
        sqrB = vpadalq_u16(sqrB, vmull_high_u8(b, b));
        sqrG = vpadalq_u16(sqrG, vmull_u8(vget_low_u8(g), vget_low_u8(g)));
        sqrG = vpadalq_u16(sqrG, vmull_high_u8(g, g));
        sqrR = vpadalq_u16(sqrR, vmull_u8(vget_low_u8(r), vget_low_u8(r)));
        sqrR = vpadalq_u16(sqrR, vmull_high_u8(r, r));

        image += 16;
        alpha += 16;
    }

    //  Remaining 0-15 pixels: full groups of 4 first.
    lc = (width % 16) / 4;
    while (lc--){
        pixel_sum_sqr_arm64_NEON_4px(
            sumB, sumG, sumR, sumA, sqrB, sqrG, sqrR,
            vld1q_u32(image), vld1q_u32(alpha)
        );
        image += 4;
        alpha += 4;
    }

    //  Last 1-3 pixels. The partial loader zero-fills the unused lanes, and a zero alpha
    //  lane is inactive, so those lanes contribute nothing.
    if (width % 4){
        PartialWordAccess_arm64_NEON loader(width * sizeof(uint32_t) % 16);
        pixel_sum_sqr_arm64_NEON_4px(
            sumB, sumG, sumR, sumA, sqrB, sqrG, sqrR,
            vreinterpretq_u32_u8(loader.load_int_no_read_past_end(image)),
            vreinterpretq_u32_u8(loader.load_int_no_read_past_end(alpha))
        );
    }

    sums.count += reduce32_arm64_NEON(sumA);
    sums.sumR += reduce32_arm64_NEON(sumR);
    sums.sumG += reduce32_arm64_NEON(sumG);
    sums.sumB += reduce32_arm64_NEON(sumB);
    sums.sqrR += reduce32_arm64_NEON(sqrR);
    sums.sqrG += reduce32_arm64_NEON(sqrG);
    sums.sqrB += reduce32_arm64_NEON(sqrB);
}
void pixel_sum_sqr_arm64_NEON(
    PixelSums& sums,
    size_t width, size_t height,
    const uint32_t* image, size_t image_bytes_per_row,
    const uint32_t* alpha, size_t alpha_bytes_per_row
){
    if (width < 4){
        pixel_sum_sqr_Default(
            sums,
            width, height,
            image, image_bytes_per_row,
            alpha, alpha_bytes_per_row
        );
        return;
    }
    if (width > 65535){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Width limit exceeded: " + std::to_string(width));
    }
    for (size_t r = 0; r < height; r++){
        pixel_sum_sqr_arm64_NEON(sums, (uint16_t)width, image, alpha);
        image = (const uint32_t*)((const char*)image + image_bytes_per_row);
        alpha = (const uint32_t*)((const char*)alpha + alpha_bytes_per_row);
    }
}



}
}
#endif
