/*  Image Filters Basic (x64 SSE4.2)
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifdef PA_AutoDispatch_08_Nehalem

#include <stdint.h>
#include <cstddef>
#include <nmmintrin.h>
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"
#include "Kernels_ImageFilter_Basic_Routines.h"

namespace PokemonAutomation{
namespace Kernels{


class ImageFilter_RgbRange_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;

public:
    ImageFilter_RgbRange_x64_SSE42(uint32_t mins, uint32_t maxs, uint32_t replacement, bool invert)
        : m_replacement(_mm_set1_epi32(replacement))
        , m_invert(invert ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_mins(_mm_set1_epi32(mins ^ 0x80808080))
        , m_maxs(_mm_set1_epi32(maxs ^ 0x80808080))
    {}

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in) const{
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        pixel = process_word(pixel);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, size_t left) const{
        PartialWordLoader_x64_SSE41 loader(left * sizeof(uint32_t));
        __m128i pixel = loader.load(in);
        pixel = process_word(pixel);
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    PA_FORCE_INLINE __m128i process_word(__m128i pixel) const{
        __m128i adj = _mm_xor_si128(pixel, _mm_set1_epi8((uint8_t)0x80));
        __m128i cmp0 = _mm_cmpgt_epi8(m_mins, adj);
        __m128i cmp1 = _mm_cmpgt_epi8(adj, m_maxs);
        cmp0 = _mm_or_si128(cmp0, cmp1);
        cmp0 = _mm_cmpeq_epi32(cmp0, _mm_setzero_si128());
        cmp0 = _mm_xor_si128(cmp0, m_invert);
        return _mm_blendv_epi8(m_replacement, pixel, cmp0);
    }

private:
    __m128i m_replacement;
    __m128i m_invert;
    __m128i m_mins;
    __m128i m_maxs;
};



void filter_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0
){
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
    filter_rbg32(image, bytes_per_row, width, height, out0, bytes_per_row0, filter0);
}
void filter2_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1
){
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_SSE42 filter1(mins1, maxs1, replacement1, invert1);
    filter2_rbg32(
        image, bytes_per_row, width, height,
        out0, bytes_per_row0, filter0,
        out1, bytes_per_row1, filter1
    );
}
void filter4_rgb32_range_x64_SSE42(
    const uint32_t* image, size_t bytes_per_row, size_t width, size_t height,
    uint32_t* out0, size_t bytes_per_row0, uint32_t mins0, uint32_t maxs0, uint32_t replacement0, bool invert0,
    uint32_t* out1, size_t bytes_per_row1, uint32_t mins1, uint32_t maxs1, uint32_t replacement1, bool invert1,
    uint32_t* out2, size_t bytes_per_row2, uint32_t mins2, uint32_t maxs2, uint32_t replacement2, bool invert2,
    uint32_t* out3, size_t bytes_per_row3, uint32_t mins3, uint32_t maxs3, uint32_t replacement3, bool invert3
){
    ImageFilter_RgbRange_x64_SSE42 filter0(mins0, maxs0, replacement0, invert0);
    ImageFilter_RgbRange_x64_SSE42 filter1(mins1, maxs1, replacement1, invert1);
    ImageFilter_RgbRange_x64_SSE42 filter2(mins2, maxs2, replacement2, invert2);
    ImageFilter_RgbRange_x64_SSE42 filter3(mins3, maxs3, replacement3, invert3);
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
