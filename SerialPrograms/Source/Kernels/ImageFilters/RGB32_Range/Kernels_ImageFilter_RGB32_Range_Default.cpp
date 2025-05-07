/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_Default.h"
#include "Kernels_ImageFilter_RGB32_Range.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{



class PixelTest_Rgb32Range_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Range_Default(
        PixelRGB32 mins, PixelRGB32 maxs
    )
        : m_shiftB(mins.u32 & 0x000000ff)
        , m_shiftG(mins.u32 & 0x0000ff00)
        , m_shiftR(mins.u32 & 0x00ff0000)
        , m_shiftA(mins.u32 & 0xff000000)
        , m_thresholdB((maxs.u32 & 0x000000ff) - m_shiftB)
        , m_thresholdG((maxs.u32 & 0x0000ff00) - m_shiftG)
        , m_thresholdR((maxs.u32 & 0x00ff0000) - m_shiftR)
        , m_thresholdA((maxs.u32 & 0xff000000) - m_shiftA)
    {}
    PA_FORCE_INLINE PixelTest_Rgb32Range_Default(
        const ToBlackWhiteRgb32RangeFilter& filter
    )
        : m_shiftB(filter.mins & 0x000000ff)
        , m_shiftG(filter.mins & 0x0000ff00)
        , m_shiftR(filter.mins & 0x00ff0000)
        , m_shiftA(filter.mins & 0xff000000)
        , m_thresholdB((filter.maxs & 0x000000ff) - m_shiftB)
        , m_thresholdG((filter.maxs & 0x0000ff00) - m_shiftG)
        , m_thresholdR((filter.maxs & 0x00ff0000) - m_shiftR)
        , m_thresholdA((filter.maxs & 0xff000000) - m_shiftA)
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE bool test_word(uint32_t pixel) const{
        bool ret = true;
        ret &= (pixel & 0x000000ff) - m_shiftB <= m_thresholdB;
        ret &= (pixel & 0x0000ff00) - m_shiftG <= m_thresholdG;
        ret &= (pixel & 0x00ff0000) - m_shiftR <= m_thresholdR;
        ret &= (pixel & 0xff000000) - m_shiftA <= m_thresholdA;
        return ret;
    }

private:
    const uint32_t m_shiftB;
    const uint32_t m_shiftG;
    const uint32_t m_shiftR;
    const uint32_t m_shiftA;
    const uint32_t m_thresholdB;
    const uint32_t m_thresholdG;
    const uint32_t m_thresholdR;
    const uint32_t m_thresholdA;
};






class ImageFilterRunner_Rgb32Range_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ImageFilterRunner_Rgb32Range_Default(
        uint32_t replacement_color, bool replace_color_within_range,
        uint32_t mins, uint32_t maxs
    )
        : m_replacement_color(replacement_color)
        , m_replace_color_within_range(replace_color_within_range ? 1 : 0)
        , m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
        , m_count(0)
    {}
    ImageFilterRunner_Rgb32Range_Default(FilterRgb32RangeFilter& filter)
        : ImageFilterRunner_Rgb32Range_Default(filter.mins, filter.maxs, filter.replacement, filter.invert)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint64_t ret = 1;
        {
            uint32_t p = pixel & 0xff000000;
            ret &= p >= m_minA;
            ret &= p <= m_maxA;
        }
        {
            uint32_t p = pixel & 0x00ff0000;
            ret &= p >= m_minR;
            ret &= p <= m_maxR;
        }
        {
            uint32_t p = pixel & 0x0000ff00;
            ret &= p >= m_minG;
            ret &= p <= m_maxG;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            ret &= p >= m_minB;
            ret &= p <= m_maxB;
        }
        m_count += ret;
        ret ^= m_replace_color_within_range;
        out[0] = ret ? pixel : m_replacement_color;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_replacement_color;
    const uint32_t m_replace_color_within_range;
    const uint32_t m_minB;
    const uint32_t m_maxB;
    const uint32_t m_minG;
    const uint32_t m_maxG;
    const uint32_t m_minR;
    const uint32_t m_maxR;
    const uint32_t m_minA;
    const uint32_t m_maxA;
    size_t m_count;
};
size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    ImageFilterRunner_Rgb32Range_Default filter(
        replacement, replace_color_within_range,
        mins, maxs
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilterRunner_Rgb32Range_Default>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}











class ToBlackWhite_RgbRange_Default{
public:
    static const size_t VECTOR_SIZE = 1;
    using Mask = size_t;

public:
    ToBlackWhite_RgbRange_Default(bool in_range_black, uint32_t mins, uint32_t maxs)
        : m_in_range_black(in_range_black ? 1 : 0)
        , m_minB(mins & 0x000000ff)
        , m_maxB(maxs & 0x000000ff)
        , m_minG(mins & 0x0000ff00)
        , m_maxG(maxs & 0x0000ff00)
        , m_minR(mins & 0x00ff0000)
        , m_maxR(maxs & 0x00ff0000)
        , m_minA(mins & 0xff000000)
        , m_maxA(maxs & 0xff000000)
        , m_count(0)
    {}
    ToBlackWhite_RgbRange_Default(ToBlackWhiteRgb32RangeFilter& filter)
        : ToBlackWhite_RgbRange_Default(filter.mins, filter.maxs, filter.in_range_black)
    {}

    PA_FORCE_INLINE size_t count() const{
        return m_count;
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        uint32_t pixel = in[0];
        uint64_t ret = 1;
        {
            uint32_t p = pixel & 0xff000000;
            ret &= p >= m_minA;
            ret &= p <= m_maxA;
        }
        {
            uint32_t p = pixel & 0x00ff0000;
            ret &= p >= m_minR;
            ret &= p <= m_maxR;
        }
        {
            uint32_t p = pixel & 0x0000ff00;
            ret &= p >= m_minG;
            ret &= p <= m_maxG;
        }
        {
            uint32_t p = pixel & 0x000000ff;
            ret &= p >= m_minB;
            ret &= p <= m_maxB;
        }
        m_count += ret;
        ret ^= m_in_range_black;
        out[0] = ret ? 0xffffffff : 0xff000000;
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        process_full(out, in);
    }

private:
    const uint32_t m_in_range_black;
    const uint32_t m_minB;
    const uint32_t m_maxB;
    const uint32_t m_minG;
    const uint32_t m_maxG;
    const uint32_t m_minR;
    const uint32_t m_maxR;
    const uint32_t m_minA;
    const uint32_t m_maxA;
    size_t m_count;
};

size_t to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_Default tester(mins, maxs);
    ToBlackWhite_Rgb32_Default<PixelTest_Rgb32Range_Default> filter(
        tester, in_range_black
    );

//    ToBlackWhite_RgbRange_Default filter(in_range_black, mins, maxs);
    filter_per_pixel(image, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_Default(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ToBlackWhite_RgbRange_Default>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}







}
}
