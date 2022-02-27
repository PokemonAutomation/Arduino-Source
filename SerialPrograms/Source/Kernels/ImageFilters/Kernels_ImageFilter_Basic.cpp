/*  Image Filters Basic
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Kernels_ImageFilter_Basic.h"

#include "Kernels/Kernels_Arch.h"
#include "Kernels_ImageFilter_Basic_Default.h"
#ifdef PA_Arch_x64_SSE42
#include "Kernels_ImageFilter_Basic_x64_SSE42.h"
#endif
#ifdef PA_Arch_x64_AVX2
#include "Kernels_ImageFilter_Basic_x64_AVX2.h"
#endif
#ifdef PA_Arch_x64_AVX512
#include "Kernels_ImageFilter_Basic_x64_AVX512.h"
#endif

namespace PokemonAutomation{
namespace Kernels{


template <typename Filter>
void filter_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, const Filter& filter0
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Filter::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        uint32_t* o0 = out0;
        size_t lc = width / VECTOR_SIZE;
        do{
            filter0.process_full(o0, in);
            in += VECTOR_SIZE;
            o0 += VECTOR_SIZE;
        }while (--lc);
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            filter0.process_partial(o0, in, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
        out0 = (uint32_t*)((const char*)out0 + bytes_per_row0);
    }while (--height);
}
template <typename Filter>
void filter2_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, const Filter& filter0,
    uint32_t* out1, size_t bytes_per_row1, const Filter& filter1
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Filter::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        uint32_t* o0 = out0;
        uint32_t* o1 = out1;
        size_t lc = width / VECTOR_SIZE;
        do{
            filter0.process_full(o0, in);
            filter1.process_full(o1, in);
            in += VECTOR_SIZE;
            o0 += VECTOR_SIZE;
            o1 += VECTOR_SIZE;
        }while (--lc);
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            filter0.process_partial(o0, in, left);
            filter1.process_partial(o1, in, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
        out0 = (uint32_t*)((const char*)out0 + bytes_per_row0);
        out1 = (uint32_t*)((const char*)out1 + bytes_per_row1);
    }while (--height);
}
template <typename Filter>
void filter4_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, const Filter& filter0,
    uint32_t* out1, size_t bytes_per_row1, const Filter& filter1,
    uint32_t* out2, size_t bytes_per_row2, const Filter& filter2,
    uint32_t* out3, size_t bytes_per_row3, const Filter& filter3
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Filter::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        uint32_t* o0 = out0;
        uint32_t* o1 = out1;
        uint32_t* o2 = out2;
        uint32_t* o3 = out3;
        size_t lc = width / VECTOR_SIZE;
        do{
            filter0.process_full(o0, in);
            filter1.process_full(o1, in);
            filter2.process_full(o2, in);
            filter3.process_full(o3, in);
            in += VECTOR_SIZE;
            o0 += VECTOR_SIZE;
            o1 += VECTOR_SIZE;
            o2 += VECTOR_SIZE;
            o3 += VECTOR_SIZE;
        }while (--lc);
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            filter0.process_partial(o0, in, left);
            filter1.process_partial(o1, in, left);
            filter2.process_partial(o2, in, left);
            filter3.process_partial(o3, in, left);
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
        out0 = (uint32_t*)((const char*)out0 + bytes_per_row0);
        out1 = (uint32_t*)((const char*)out1 + bytes_per_row1);
        out2 = (uint32_t*)((const char*)out2 + bytes_per_row2);
        out3 = (uint32_t*)((const char*)out3 + bytes_per_row3);
    }while (--height);
}




void filter_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
#if 0
#elif defined PA_Arch_x64_AVX512
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
#elif defined PA_Arch_x64_AVX2
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
#elif defined PA_Arch_x64_SSE42
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
#else
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
#endif
    filter_rbg32(image, bytes_per_row, width, height, out0, bytes_per_row0, filter0);
}
void filter2_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
#if 0
#elif defined PA_Arch_x64_AVX512
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX512 filter1(mins1, maxs1, replacement1, invert1);
#elif defined PA_Arch_x64_AVX2
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX2 filter1(mins1, maxs1, replacement1, invert1);
#elif defined PA_Arch_x64_SSE42
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_SSE42 filter1(mins1, maxs1, replacement1, invert1);
#else
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_Default filter1(mins1, maxs1, replacement1, invert1);
#endif
    filter2_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1
    );
}
void filter4_rgb32_range(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
#if 0
#elif defined PA_Arch_x64_AVX512
    ImageFilter_RgbRange_x64_AVX512 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX512 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_AVX512 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_AVX512 filter3(mins3, maxs3, replacement3, invert3);
#elif defined PA_Arch_x64_AVX2
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX2 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_AVX2 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_AVX2 filter3(mins3, maxs3, replacement3, invert3);
#elif defined PA_Arch_x64_SSE42
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_SSE42 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_SSE42 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_SSE42 filter3(mins3, maxs3, replacement3, invert3);
#else
    ImageFilter_RgbRange_Default filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_Default filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_Default filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_Default filter3(mins3, maxs3, replacement3, invert3);
#endif
    filter4_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1,
        out2, bytes_per_row2, filter3,
        out3, bytes_per_row3, filter1
    );
}





}
}
