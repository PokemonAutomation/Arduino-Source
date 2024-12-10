/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_Routines_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_Routines_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels_ImageFilter_Basic.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{

// Runner interface:
// - static size_t Runner::VECTOR_SIZE, how many uint32_t in an SIMD vector. Note one pixel is one uint32_t.
// - Runner::process_full(uint32_t* out, const uint32_t* in), process a full vector: replace pixels in
//   or out of a specific color range with a particular color.
// - Runner::process_partial(uint32_t* out, const uint32_t* in, size_t left), process a partial vector:
//   work only on the first `left` pixels.
template <typename Runner>
PA_FORCE_INLINE void filter_per_pixel(
    const uint32_t* image, size_t in_bytes_per_row, size_t width, size_t height,
    Runner& filter, uint32_t* out, size_t out_bytes_per_row
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Runner::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        uint32_t* o0 = out;
        size_t lc = width / VECTOR_SIZE;
        while (lc--){
            filter.process_full(o0, in);
            in += VECTOR_SIZE;
            o0 += VECTOR_SIZE;
        }
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            filter.process_partial(o0, in, left);
        }
        image = (const uint32_t*)((const char*)image + in_bytes_per_row);
        out = (uint32_t*)((const char*)out + out_bytes_per_row);
    }while (--height);
}


template <typename Runner>
PA_FORCE_INLINE void filter_per_pixel(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    if (width == 0 || height == 0){
        return;
    }

    FixedLimitVector<Runner> entries(filter_count);
    for (size_t c = 0; c < filter_count; c++){
        entries.emplace_back(filter[c].mins, filter[c].maxs, filter[c].replacement, filter[c].invert);
    }

    const size_t VECTOR_SIZE = Runner::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        size_t shift = 0;
        size_t lc = width / VECTOR_SIZE;
        while (lc--){
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_full(filter[c].data + shift, in);
            }
            in += VECTOR_SIZE;
            shift += VECTOR_SIZE;
        }
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_partial(filter[c].data + shift, in, left);
            }
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
        for (size_t c = 0; c < filter_count; c++){
            filter[c].data = (uint32_t*)((const char*)filter[c].data + filter[c].bytes_per_row);
        }
    }while (--height);
    for (size_t c = 0; c < filter_count; c++){
        filter[c].pixels_in_range = entries[c].count();
    }
}


template <typename Runner>
PA_FORCE_INLINE void to_blackwhite_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    if (width == 0 || height == 0){
        return;
    }

    FixedLimitVector<Runner> entries(filter_count);
    for (size_t c = 0; c < filter_count; c++){
        entries.emplace_back(filter[c].mins, filter[c].maxs, filter[c].in_range_black);
    }

    const size_t VECTOR_SIZE = Runner::VECTOR_SIZE;
    do{
        const uint32_t* in = image;
        size_t shift = 0;
        size_t lc = width / VECTOR_SIZE;
        while (lc--){
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_full(filter[c].data + shift, in);
            }
            in += VECTOR_SIZE;
            shift += VECTOR_SIZE;
        }
        size_t left = width % VECTOR_SIZE;
        if (left != 0){
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_partial(filter[c].data + shift, in, left);
            }
        }
        image = (const uint32_t*)((const char*)image + bytes_per_row);
        for (size_t c = 0; c < filter_count; c++){
            filter[c].data = (uint32_t*)((const char*)filter[c].data + filter[c].bytes_per_row);
        }
    }while (--height);
    for (size_t c = 0; c < filter_count; c++){
        filter[c].pixels_in_range = entries[c].count();
    }
}


















}
}
#endif
