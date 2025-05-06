/*  Image Filters RGB32 Euclidean
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Euclidean.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbEuclidean_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ImageFilter_RgbEuclidean_x64_AVX2(
        uint32_t replacement, bool replace_color_within_range,
        uint32_t expected, double max_euclidean_distance
    )
        : m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
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
    uint32_t replacement, bool replace_color_within_range,
    uint32_t expected, double max_euclidean_distance
){
    ImageFilter_RgbEuclidean_x64_AVX2 filter(
        replacement, replace_color_within_range,
        expected, max_euclidean_distance
    );
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}




}
}
#endif
