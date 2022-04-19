/*  Image Filters Basic (x64 AVX2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_13_Haswell

#include <stdint.h>
#include <cstddef>
#include <immintrin.h>
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
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
        __m256i pixel = _mm256_loadu_si256((const __m256i*)in);
        pixel = process_word(pixel);
        _mm256_storeu_si256((__m256i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        PartialWordAccess32_x64_AVX2 loader(left);
        __m256i pixel = loader.load(in);
        pixel = process_word(pixel);
        loader.store(out, pixel);
    }

private:
    PA_FORCE_INLINE __m256i process_word(__m256i pixel) const{
        __m256i adj = _mm256_xor_si256(pixel, _mm256_set1_epi8((uint8_t)0x80));
        __m256i cmp0 = _mm256_cmpgt_epi8(m_mins, adj);
        __m256i cmp1 = _mm256_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm256_or_si256(cmp0, cmp1);
        cmp0 = _mm256_cmpeq_epi32(cmp0, _mm256_setzero_si256());
        cmp0 = _mm256_xor_si256(cmp0, m_invert);
        return _mm256_blendv_epi8(m_replacement, pixel, cmp0);
    }

private:
    __m256i m_replacement;
    __m256i m_invert;
    __m256i m_mins;
    __m256i m_maxs;
};



void filter_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, out0, bytes_per_row0, filter0);
}
void filter2_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX2 filter1(mins1, maxs1, replacement1, invert1);
    filter2_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1
    );
}
void filter4_rgb32_range_x64_AVX2(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
    ImageFilter_RgbRange_x64_AVX2 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_AVX2 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_AVX2 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_AVX2 filter3(mins3, maxs3, replacement3, invert3);
    filter4_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1,
        out2, bytes_per_row2, filter3,
        out3, bytes_per_row3, filter1
    );
}



}
}
#endif
