/*  Image Filters RGB32 Range
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines_x64_AVX2.h"
#include "Kernels_ImageFilter_RGB32_Range.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{





class PixelTest_Rgb32Range_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    PA_FORCE_INLINE PixelTest_Rgb32Range_x64_AVX2(
        uint32_t mins, uint32_t maxs
    )
        : m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
    {}
    PA_FORCE_INLINE PixelTest_Rgb32Range_x64_AVX2(
        const ToBlackWhiteRgb32RangeFilter& filter
    )
        : m_mins(_mm256_set1_epi32(filter.mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(filter.maxs ^ 0x80808080))
    {}

    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m256i test_word(__m256i pixel) const{
        __m256i adj = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, adj);
        __m256i cmp1 = _mm256_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        return _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
    }

private:
    const __m256i m_mins;
    const __m256i m_maxs;
};







size_t filter_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_x64_AVX2 tester(mins, maxs);
    FilterImage_Rgb32_x64_AVX2<PixelTest_Rgb32Range_x64_AVX2> filter(
        tester,
        replacement, replace_color_within_range
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
size_t to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
){
    PixelTest_Rgb32Range_x64_AVX2 tester(mins, maxs);
    ToBlackWhite_Rgb32_x64_AVX2<PixelTest_Rgb32Range_x64_AVX2> filter(
        tester, in_range_black
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
#endif
