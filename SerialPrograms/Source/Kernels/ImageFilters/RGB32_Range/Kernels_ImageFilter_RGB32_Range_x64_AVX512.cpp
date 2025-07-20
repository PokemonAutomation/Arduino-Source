/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX512.h"
#include "Kernels_ImageFilter_RGB32_Range.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{





class PixelTest_Rgb32Range_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Range_x64_AVX512(
        uint32_t mins, uint32_t maxs
    )
        : m_shift(_mm512_set1_epi32(mins))
        , m_threshold(_mm512_sub_epi8(_mm512_set1_epi32(maxs), m_shift))
    {}
    PA_FORCE_INLINE PixelTest_Rgb32Range_x64_AVX512(
        const ToBlackWhiteRgb32RangeFilter& filter
    )
        : m_shift(_mm512_set1_epi32(filter.mins))
        , m_threshold(_mm512_sub_epi8(_mm512_set1_epi32(filter.maxs), m_shift))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __mmask16 test_word(__m512i pixel) const{
        pixel = _mm512_sub_epi8(pixel, m_shift);
        __mmask64 cmp64 = _mm512_cmple_epu8_mask(pixel, m_threshold);
        __m512i mask = _mm512_movm_epi8(cmp64);
        return _mm512_cmpeq_epi32_mask(mask, _mm512_set1_epi32(-1));
    }

private:
    const __m512i m_shift;
    const __m512i m_threshold;
};





size_t filter_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_x64_AVX512 tester(mins, maxs);
    FilterImage_Rgb32_x64_AVX512<PixelTest_Rgb32Range_x64_AVX512> filter(
        tester,
        replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_range_x64_AVX512(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_x64_AVX512 tester(mins, maxs);
    ToBlackWhite_Rgb32_x64_AVX512<PixelTest_Rgb32Range_x64_AVX512> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
















}
}
#endif
