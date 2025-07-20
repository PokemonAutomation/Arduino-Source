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
        uint32_t mins, uint32_t maxs
    )
        : m_shiftB(mins & 0x000000ff)
        , m_shiftG(mins & 0x0000ff00)
        , m_shiftR(mins & 0x00ff0000)
        , m_shiftA(mins & 0xff000000)
        , m_thresholdB((maxs & 0x000000ff) - m_shiftB)
        , m_thresholdG((maxs & 0x0000ff00) - m_shiftG)
        , m_thresholdR((maxs & 0x00ff0000) - m_shiftR)
        , m_thresholdA((maxs & 0xff000000) - m_shiftA)
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






size_t filter_rgb32_range_Default(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_Default tester(mins, maxs);
    FilterImage_Rgb32_Default<PixelTest_Rgb32Range_Default> filter(
        tester,
        replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
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
    filter_per_pixel(image, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
