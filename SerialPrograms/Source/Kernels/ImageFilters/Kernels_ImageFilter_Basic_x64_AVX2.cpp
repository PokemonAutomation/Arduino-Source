/*  Image Filters Basic (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_13_Haswell

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;

public:
    ImageFilter_RgbRange_x64_AVX2(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
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
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left){
        PartialWordAccess32_x64_AVX2 loader(left);
        __m256i pixel = loader.load(in);
        pixel = process_word(pixel);
        loader.store(out, pixel);
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



void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    size_t& count0, uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, filter0, out0, bytes_per_row0);
    count0 = filter0.count();
}



void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    FilterRgb32RangeFilter* filter, size_t filter_count
){
    filter_rbg32<ImageFilter_RgbRange_x64_AVX2>(
        image, bytes_per_row, width, height, filter, filter_count
    );
}






}
}
#endif
