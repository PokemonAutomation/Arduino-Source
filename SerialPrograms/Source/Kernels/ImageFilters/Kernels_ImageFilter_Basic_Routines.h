/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_Basic_Routines_H
#define PokemonAutomation_Kernels_ImageFilter_Basic_Routines_H

#include <stddef.h>
#include <stdint.h>
#include "Common/Compiler.h"

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

    //  Less than vector width. No need for steady-state loop.
    if (width < VECTOR_SIZE){
        typename Runner::Mask mask(width);
        do{
            const uint32_t* in = image;
            uint32_t* o0 = out;
            filter.process_partial(o0, in, mask);
            image = (const uint32_t*)((const char*)image + in_bytes_per_row);
            out = (uint32_t*)((const char*)out + out_bytes_per_row);
        }while (--height);
        return;
    }

    //  Divisible by vector width. No need for peel iteration.
    size_t left = width % VECTOR_SIZE;
    if (left == 0){
        do{
            const uint32_t* in = image;
            uint32_t* o0 = out;
            size_t lc = width / VECTOR_SIZE;
            do{
                filter.process_full(o0, in);
                in += VECTOR_SIZE;
                o0 += VECTOR_SIZE;
            }while (--lc);
            image = (const uint32_t*)((const char*)image + in_bytes_per_row);
            out = (uint32_t*)((const char*)out + out_bytes_per_row);
        }while (--height);
        return;
    }

    //  Need both steady-state and peel iteration.
    {
        do{
            const uint32_t* in = image;
            uint32_t* o0 = out;
            size_t lc = width / VECTOR_SIZE;
            do{
                filter.process_full(o0, in);
                in += VECTOR_SIZE;
                o0 += VECTOR_SIZE;
            }while (--lc);
            filter.process_partial(o0, in, left);
            image = (const uint32_t*)((const char*)image + in_bytes_per_row);
            out = (uint32_t*)((const char*)out + out_bytes_per_row);
        }while (--height);
    }
}
}
}
#endif
