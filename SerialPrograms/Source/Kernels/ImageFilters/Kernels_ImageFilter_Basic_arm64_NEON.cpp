/*  Image Filters Basic (arm64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <stdint.h>
#include <cstddef>
#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_arm64_NEON.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{

//TODO: impl this file

size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool invert
);

void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
);

size_t filter_rgb32_euclidean_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement, bool invert
);

size_t to_blackwhite_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
);

void to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
);

class ImageFilter_RgbRange_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;

public:
    ImageFilter_RgbRange_arm64_NEON(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
    {}

    PA_FORCE_INLINE size_t count() const{
        return 0;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
    }

private:
    PA_FORCE_INLINE int process_word(int pixel){
        return 0;
    }

private:
};



size_t filter_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool invert
){
    return filter_rgb32_range_Default(in, in_bytes_per_row, width, height, out,
        out_bytes_per_row, mins, maxs, replacement, invert);
    // ImageFilter_RgbRange_arm64_NEON filter(mins, maxs, replacement, invert);
    // filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    // return filter.count();
}
void filter_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_rgb32_range_Default(image, bytes_per_row, width, height, filter, filter_count);
    // filter_per_pixel<ImageFilter_RgbRange_arm64_NEON>(
    //     image, bytes_per_row, width, height, filter, filter_count
    // );
}





class ImageFilter_RgbEuclidean_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;

public:
    ImageFilter_RgbEuclidean_arm64_NEON(uint32_t expected, double max_euclidean_distance, uint32_t replacement, bool invert)
    {}

    PA_FORCE_INLINE size_t count() const{
        return 0;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
    }

private:
    PA_FORCE_INLINE int process_word(int pixel){
        return 0;
    }

private:

};
size_t filter_rgb32_euclidean_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement, bool invert
){
    return filter_rgb32_euclidean_Default(in, in_bytes_per_row, width, height,
        out, out_bytes_per_row, expected, max_euclidean_distance, replacement, invert);
    // ImageFilter_RgbEuclidean_arm64_NEON filter(expected, max_euclidean_distance, replacement, invert);
    // filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    // return filter.count();
}








class ToBlackWhite_RgbRange_arm64_NEON{
public:
    static const size_t VECTOR_SIZE = 4;

public:
    ToBlackWhite_RgbRange_arm64_NEON(uint32_t mins, uint32_t maxs, bool in_range_black)
    {}

    PA_FORCE_INLINE size_t count() const{
        return 0;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
    }

private:
    PA_FORCE_INLINE int process_word(int pixel){
        return 0;
    }

private:
};



size_t to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    return to_blackwhite_rgb32_range_Default(in, in_bytes_per_row, width, height,
        out, out_bytes_per_row, mins, maxs, in_range_black);
    // ToBlackWhite_RgbRange_arm64_NEON filter(mins, maxs, in_range_black);
    // filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    // return filter.count();
}
void to_blackwhite_rgb32_range_arm64_NEON(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    return to_blackwhite_rgb32_range_Default(image, bytes_per_row, width, height,
        filter, filter_count);
    // to_blackwhite_rbg32<ToBlackWhite_RgbRange_arm64_NEON>(
    //     image, bytes_per_row, width, height, filter, filter_count
    // );
}








}
}
#endif
