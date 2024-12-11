/*  Image Filters Basic (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilterByMask_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ImageFilterByMask_x64_AVX2(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(invert ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm256_setzero_si256())
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
        cmp0 = _mm256_xor_si256(cmp0, m_invert);
        return _mm256_blendv_epi8(m_replacement, pixel, cmp0);
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
    uint32_t mins, uint32_t maxs,
    uint32_t replacement, bool invert
){
    ImageFilterByMask_x64_AVX2 filter(mins, maxs, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_per_pixel<ImageFilterByMask_x64_AVX2>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}





class ImageFilter_RgbEuclidean_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ImageFilter_RgbEuclidean_x64_AVX2(uint32_t expected, double max_euclidean_distance, uint32_t replacement, bool invert)
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(invert ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_expected_ag(_mm256_set1_epi32((expected >> 8) & 0x000000ff))
        , m_expected_rb(_mm256_set1_epi32(expected & 0x00ff00ff))
        , m_distance_squared(_mm256_set1_epi32((uint32_t)(max_euclidean_distance * max_euclidean_distance)))
        , m_count(_mm256_setzero_si256())
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
        __m256i ag = _mm256_and_si256(_mm256_srli_epi16(pixel, 8), _mm256_set1_epi32(0x000000ff));
        __m256i rb = _mm256_and_si256(pixel, _mm256_set1_epi32(0x00ff00ff));

        ag = _mm256_sub_epi16(ag, m_expected_ag);
        rb = _mm256_sub_epi16(rb, m_expected_rb);

        __m256i g = _mm256_mullo_epi16(ag, ag);
        rb = _mm256_mullo_epi16(rb, rb);
        __m256i r = _mm256_srli_epi32(rb, 16);
        __m256i b = _mm256_and_si256(rb, _mm256_set1_epi32(0x0000ffff));

        __m256i sum_sqr = _mm256_add_epi32(r, g);
        sum_sqr = _mm256_add_epi32(sum_sqr, b);

        __m256i cmp = _mm256_cmpgt_epi32(m_distance_squared, sum_sqr);

        m_count = _mm256_sub_epi32(m_count, cmp);
        cmp = _mm256_xor_si256(cmp, m_invert);
        return _mm256_blendv_epi8(m_replacement, pixel, cmp);
    }

private:
    const __m256i m_replacement;
    const __m256i m_invert;
    const __m256i m_expected_ag;
    const __m256i m_expected_rb;
    const __m256i m_distance_squared;
    __m256i m_count;
};
size_t filter_rgb32_euclidean_x64_AVX2(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    uint32_t expected, double max_euclidean_distance,
    uint32_t replacement, bool invert
){
    ImageFilter_RgbEuclidean_x64_AVX2 filter(expected, max_euclidean_distance, replacement, invert);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}






class ToBlackWhite_RgbRange_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ToBlackWhite_RgbRange_x64_AVX2(uint32_t mins, uint32_t maxs, bool in_range_black)
        : m_in_range_black(in_range_black ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_mins(_mm256_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm256_set1_epi32(maxs ^ 0x80808080))
        , m_count(_mm256_setzero_si256())
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
    uint32_t mins, uint32_t maxs, bool in_range_black
){
    ToBlackWhite_RgbRange_x64_AVX2 filter(mins, maxs, in_range_black);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}
void to_blackwhite_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    ToBlackWhiteRgb32RangeFilter* filter, size_t filter_count
){
    to_blackwhite_rbg32<ToBlackWhite_RgbRange_x64_AVX2>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}










}
}
#endif
