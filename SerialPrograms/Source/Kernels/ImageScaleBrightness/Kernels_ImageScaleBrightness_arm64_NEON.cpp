/*  Scale Brightness (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <stddef.h>
#include <stdint.h>
#include <arm_neon.h>
#include <algorithm>
#include "Common/Compiler.h"

#include <iostream>
using std::cout, std::endl;

namespace PokemonAutomation{
namespace Kernels{

// #define CHECK_ACCURACY
namespace{

PA_FORCE_INLINE void scale_brightness_arm64_NEON_four_pixels_per_channel_scale(
    size_t width, uint32_t* image,
    float r_scale, float g_scale, float b_scale
){
    #ifdef CHECK_ACCURACY
    {
        uint32_t pixel = image[0];
        uint32_t r = (pixel >> 16) & 0x000000ff;
        uint32_t g = (pixel >> 8) & 0x000000ff;
        uint32_t b = pixel & 0x000000ff;
        cout << "r g b " << r << " " << g << " " << b << endl;
    }
    #endif

    const uint32x4_t mask_u32x4 = vmovq_n_u32(0xff);
    const uint32x4_t alpha_mask_u32x4 = vmovq_n_u32(0xff000000);

    const uint32x4_t max_brightness_u32x4 = vmovq_n_u32(255);

    const size_t width_aligned = width - width % 4;
    for (size_t c = 0; c < width_aligned; c+=4){
        // Load four pixels
        uint32x4_t vec_u32x4 = vld1q_u32(&image[c]);
        // Extract each color channel of the four pixels
        uint32x4_t b_u32x4 = vandq_u32(vec_u32x4, mask_u32x4);
        uint32x4_t g_u32x4 = vandq_u32(vshrq_n_u32(vec_u32x4, 8), mask_u32x4);
        uint32x4_t r_u32x4 = vandq_u32(vshrq_n_u32(vec_u32x4, 16), mask_u32x4);
        uint32x4_t a_u32x4 = vandq_u32(vec_u32x4, alpha_mask_u32x4);

        // Convert them to float32
        float32x4_t b_f32x4 = vcvtq_f32_u32(b_u32x4);
        float32x4_t g_f32x4 = vcvtq_f32_u32(g_u32x4);
        float32x4_t r_f32x4 = vcvtq_f32_u32(r_u32x4);

        // Scale color channels
        b_f32x4 = vmulq_n_f32(b_f32x4, b_scale);
        g_f32x4 = vmulq_n_f32(g_f32x4, g_scale);
        r_f32x4 = vmulq_n_f32(r_f32x4, r_scale);

        // Convert each float32 back to uint32
        b_u32x4 = vcvtq_u32_f32(b_f32x4);
        g_u32x4 = vcvtq_u32_f32(g_f32x4);
        r_u32x4 = vcvtq_u32_f32(r_f32x4);

        // Clamp against 255
        b_u32x4 = vminq_u32(b_u32x4, max_brightness_u32x4);
        g_u32x4 = vminq_u32(g_u32x4, max_brightness_u32x4);
        r_u32x4 = vminq_u32(r_u32x4, max_brightness_u32x4);

        // Add channels back to form four pixels
        // shift g channels left by 8, then combine it with b channels
        uint32x4_t gb_u32x4 = vsliq_n_u32(b_u32x4, g_u32x4, 8);
        // shift r channels left by 16, then combine with gb channels
        uint32x4_t rgb_u32x4 = vsliq_n_u32(gb_u32x4, r_u32x4, 16);
        vec_u32x4 = vorrq_s32(a_u32x4, rgb_u32x4);
        vst1q_u32(&image[c], vec_u32x4);
    }

    for(size_t c = width_aligned; c < width; c++){
        uint32_t pixel = image[c];
        float r = (float)((pixel >> 16) & 0x000000ff);
        float g = (float)((pixel >> 8) & 0x000000ff);
        float b = (float)(pixel & 0x000000ff);
        r *= r_scale;
        g *= g_scale;
        b *= b_scale;
        uint32_t r_u32 = std::min((uint32_t)r, (uint32_t)255);
        uint32_t g_u32 = std::min((uint32_t)g, (uint32_t)255);
        uint32_t b_u32 = std::min((uint32_t)b, (uint32_t)255);

        pixel &= 0xff000000;
        pixel |= r_u32 << 16;
        pixel |= g_u32 << 8;
        pixel |= b_u32;

        image[c] = pixel;
    }

    #ifdef CHECK_ACCURACY
    {
        uint32_t pixel = image[0];
        uint32_t r = (pixel >> 16) & 0x000000ff;
        uint32_t g = (pixel >> 8) & 0x000000ff;
        uint32_t b = pixel & 0x000000ff;
        cout << std::dec << "After: r g b " << r << " " << g << " " << b << endl;
    }
    #endif

}

}

void scale_brightness_arm64_NEON(
    size_t width, size_t height,
    uint32_t* image, size_t bytes_per_row,
    float scaleR, float scaleG, float scaleB
){
    if (width == 0 || height == 0){
        return;
    }
    scaleR = std::max(scaleR, 0.0f);
    scaleG = std::max(scaleG, 0.0f);
    scaleB = std::max(scaleB, 0.0f);

    for (uint16_t r = 0; r < height; r++){
        scale_brightness_arm64_NEON_four_pixels_per_channel_scale(width, image, scaleR, scaleG, scaleB);
        image = (uint32_t*)((char*)image + bytes_per_row);

        #ifdef CHECK_ACCURACY
        break;
        #endif
    }
}



}
}
#endif
