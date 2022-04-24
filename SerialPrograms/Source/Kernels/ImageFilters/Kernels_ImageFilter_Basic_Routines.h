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
#include "Common/Cpp/FixedLimitVector.tpp"
#include "Kernels_ImageFilter_Basic.h"

namespace PokemonAutomation{
namespace Kernels{


template <typename Runner>
PA_FORCE_INLINE void filter_rbg32(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    Runner& filter0, uint32_t* out0, size_t bytes_per_row0
){
    if (width == 0 || height == 0){
        return;
    }
    const size_t VECTOR_SIZE = Runner::VECTOR_SIZE;
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


template <typename Runner>
PA_FORCE_INLINE void filter_rbg32(
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
        do{
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_full(filter[c].data + shift, in);
            }
            in += VECTOR_SIZE;
            shift += VECTOR_SIZE;
        }while (--lc);
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
        do{
            for (size_t c = 0; c < filter_count; c++){
                entries[c].process_full(filter[c].data + shift, in);
            }
            in += VECTOR_SIZE;
            shift += VECTOR_SIZE;
        }while (--lc);
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
