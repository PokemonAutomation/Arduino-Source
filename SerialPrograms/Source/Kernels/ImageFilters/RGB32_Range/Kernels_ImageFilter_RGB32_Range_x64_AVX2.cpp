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
#include "Kernels_ImageFilter_RGB32_Range.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{


class ImageFilterRunner_Rgb32Range_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ImageFilterRunner_Rgb32Range_x64_AVX2(
        uint32_t replacement, bool replace_color_within_range,
        uint32_t mins, uint32_t maxs
    )
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm256_setzero_si256())
    {}
    ImageFilterRunner_Rgb32Range_x64_AVX2(FilterRgb32RangeFilter& filter)
        : ImageFilterRunner_Rgb32Range_x64_AVX2(filter.mins, filter.maxs, filter.replacement, filter.invert)
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce_add32_x64_AVX2(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m256i pixel = _mm256_loadu_si256((const __m256i*)in);
        __m256i in_range_pixels = process_word(pixel);
        m_count = _mm256_sub_epi32(m_count, in_range_pixels);
        _mm256_storeu_si256((__m256i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m256i pixel = mask.load_i32(in);
        __m256i in_range_pixels = process_word(pixel);
        in_range_pixels = _mm256_and_si256(in_range_pixels, mask.mask());
        m_count = _mm256_sub_epi32(m_count, in_range_pixels);
        mask.store(out, pixel);
    }

private:
    PA_FORCE_INLINE __m256i process_word(__m256i& pixel){
        __m256i adj = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, adj);
        __m256i cmp1 = _mm256_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        cmp0 = _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
        pixel = _mm256_blendv_epi8(m_replacement, pixel, _mm256_xor_si256(cmp0, m_invert));
        return cmp0;
    }

private:
    const __m256i m_replacement;
    const __m256i m_invert;
    const __m256i m_mins;
    const __m256i m_maxs;
    __m256i m_count;
};



size_t filter_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t replacement, bool replace_color_within_range,
    uint32_t mins, uint32_t maxs
){
    ImageFilterRunner_Rgb32Range_x64_AVX2 filter(
        replacement, replace_color_within_range,
        mins, maxs
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilterRunner_Rgb32Range_x64_AVX2>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}







class ToBlackWhite_RgbRange_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ToBlackWhite_RgbRange_x64_AVX2(bool in_range_black, uint32_t mins, uint32_t maxs)
        : m_in_range_black(in_range_black ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm256_setzero_si256())
    {}
    ToBlackWhite_RgbRange_x64_AVX2(ToBlackWhiteRgb32RangeFilter& filter)
        : ToBlackWhite_RgbRange_x64_AVX2(filter.mins, filter.maxs, filter.in_range_black)
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce_add32_x64_AVX2(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m256i pixel = _mm256_loadu_si256((const __m256i*)in);
        pixel = process_word(pixel);
        _mm256_storeu_si256((__m256i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m256i pixel = mask.load_i32(in);
        pixel = process_word(pixel);
        mask.store(out, pixel);
    }

private:
    PA_FORCE_INLINE __m256i process_word(__m256i pixel){
        __m256i adj = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, adj);
        __m256i cmp1 = _mm256_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        cmp0 = _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
        m_count = _mm256_sub_epi32(m_count, cmp0);
        cmp0 = _mm256_xor_si256(cmp0, m_in_range_black);
        return _mm256_or_si256(cmp0, _mm256_set1_epi32(0xff000000));
    }

private:
    const __m256i m_in_range_black;
    const __m256i m_mins;
    const __m256i m_maxs;
    __m256i m_count;
};

size_t to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t mins, uint32_t maxs
){
    ToBlackWhite_RgbRange_x64_AVX2 filter(in_range_black, mins, maxs);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ToBlackWhite_RgbRange_x64_AVX2>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





}
}
#endif
