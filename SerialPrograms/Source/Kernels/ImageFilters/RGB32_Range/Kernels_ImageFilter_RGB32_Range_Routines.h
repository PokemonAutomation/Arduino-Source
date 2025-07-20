/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_ImageFilter_RGB32_Range_Routines_H
#define PokemonAutomation_Kernels_ImageFilter_RGB32_Range_Routines_H

#include "Common/Compiler.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Kernels_ImageFilter_RGB32_Range.h"

namespace PokemonAutomation{
namespace Kernels{



template <typename Runner>
PA_FORCE_INLINE void filter_per_pixel(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filters, size_t filter_count
){
    if (width == 0 || height == 0){
        return;
    }

//    cout << "filter_per_pixel(" << filter_count << "): " << width << " x " << height << endl;

    FixedLimitVector<Runner> entries(filter_count);
    for (size_t c = 0; c < filter_count; c++){
        FilterRgb32RangeFilter& filter = filters[c];
        entries.emplace_back(filter);
    }

    const size_t VECTOR_SIZE = Runner::VECTOR_SIZE;
    do{
        //  Less than vector width. No need for steady-state loop.
        if (width < VECTOR_SIZE){
            typename Runner::Mask mask(width);
            do{
                const uint32_t* in = image;
                for (size_t c = 0; c < filter_count; c++){
                    entries[c].process_partial(filters[c].data, in, mask);
                }
                image = (const uint32_t*)((const char*)image + bytes_per_row);
                for (size_t c = 0; c < filter_count; c++){
                    FilterRgb32RangeFilter& filter = filters[c];
                    filter.data = (uint32_t*)((const char*)filter.data + filter.bytes_per_row);
                }
            }while (--height);
            break;
        }

        //  Divisible by vector width. No need for peel loop.
        size_t left = width % VECTOR_SIZE;
        if (left == 0){
            do{
                const uint32_t* in = image;
                size_t shift = 0;
                size_t lc = width / VECTOR_SIZE;
                do{
                    for (size_t c = 0; c < filter_count; c++){
                        entries[c].process_full(filters[c].data + shift, in);
                    }
                    in += VECTOR_SIZE;
                    shift += VECTOR_SIZE;
                }while (--lc);
                image = (const uint32_t*)((const char*)image + bytes_per_row);
                for (size_t c = 0; c < filter_count; c++){
                    FilterRgb32RangeFilter& filter = filters[c];
                    filter.data = (uint32_t*)((const char*)filter.data + filter.bytes_per_row);
                }
            }while (--height);
            break;
        }

        //  Need both steady-state and peel loops.
        {
            typename Runner::Mask mask(left);
            do{
                const uint32_t* in = image;
                size_t shift = 0;
                size_t lc = width / VECTOR_SIZE;
                do{
                    for (size_t c = 0; c < filter_count; c++){
                        entries[c].process_full(filters[c].data + shift, in);
                    }
                    in += VECTOR_SIZE;
                    shift += VECTOR_SIZE;
                }while (--lc);
                for (size_t c = 0; c < filter_count; c++){
                    entries[c].process_partial(filters[c].data + shift, in, mask);
                }
                image = (const uint32_t*)((const char*)image + bytes_per_row);
                for (size_t c = 0; c < filter_count; c++){
                    FilterRgb32RangeFilter& filter = filters[c];
                    filter.data = (uint32_t*)((const char*)filter.data + filter.bytes_per_row);
                }
            }while (--height);
            break;
        }
    }while (false);

    for (size_t c = 0; c < filter_count; c++){
        filters[c].pixels_in_range = entries[c].count();
    }
}




}
}
#endif
