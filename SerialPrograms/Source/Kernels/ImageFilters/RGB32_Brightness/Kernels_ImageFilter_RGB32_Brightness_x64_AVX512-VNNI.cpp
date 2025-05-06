/*  Image Filters RGB32 Brightness
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_19_IceLake

#include <immintrin.h>
#include "Kernels/ImageFilters/Kernels_ImageFilter_Basic_Routines.h"
#include "Kernels_ImageFilter_RGB32_Brightness.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{


namespace{

struct PartialWordMask{
    __mmask16 m;

    PA_FORCE_INLINE PartialWordMask(size_t left)
        : m(((__mmask16)1 << left) - 1)
    {}
};

}




class ToBlackWhite_RgbBrightness_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ToBlackWhite_RgbBrightness_x64_AVX512(
        bool in_range_black,
        uint32_t min_brightness, uint32_t max_brightness
    )
        : m_in_range_black(in_range_black ? 0xffff : 0)
        , m_min_brightness(_mm512_set1_epi32(min_brightness))
        , m_threshold(_mm512_set1_epi32(max_brightness - min_brightness))
        , m_count(_mm512_setzero_si512())
    {}

    PA_FORCE_INLINE size_t count() const{
        return _mm512_reduce_add_epi32(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m512i pixel = _mm512_loadu_si512((const __m512i*)in);
        __mmask16 in_range_pixels = process_word(pixel);
        m_count = _mm512_mask_sub_epi32(m_count, in_range_pixels, m_count, _mm512_set1_epi32(-1));
        _mm512_storeu_si512((__m512i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m512i pixel = _mm512_maskz_loadu_epi32(mask.m, in);
        __mmask16 in_range_pixels = process_word(pixel);
        in_range_pixels &= mask.m;
        m_count = _mm512_mask_sub_epi32(m_count, in_range_pixels, m_count, _mm512_set1_epi32(-1));
        _mm512_mask_storeu_epi32(out, mask.m, pixel);
    }

private:
    //  Process the pixel in-place.
    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __mmask16 process_word(__m512i& pixel) const{
//        __m512i r = _mm512_and_si512(_mm512_srli_epi32(pixel, 16), _mm512_set1_epi32(0x000000ff));
//        __m512i g = _mm512_and_si512(_mm512_srli_epi32(pixel,  8), _mm512_set1_epi32(0x000000ff));
//        __m512i b = _mm512_and_si512(pixel,                        _mm512_set1_epi32(0x000000ff));

        //  Remove the alpha channel.
        pixel = _mm512_and_si512(pixel, _mm512_set1_epi32(0x00ffffff));

        //  Horizontally sum up 4 x 8-bit integers in each 32-bit pixel.
        pixel = _mm512_dpbusd_epi32(_mm512_setzero_si512(), pixel, _mm512_set1_epi8(1));

        //  Find the ones that are in range.
        pixel = _mm512_sub_epi32(pixel, m_min_brightness);
        __mmask16 cmp16 = _mm512_cmple_epu32_mask(pixel, m_threshold);

        //  Set to black or white.
        pixel = _mm512_mask_blend_epi32(
            cmp16 ^ m_in_range_black,
            _mm512_set1_epi32(0xff000000),
            _mm512_set1_epi32(-1)
        );

        return cmp16;
    }

private:
    const __mmask16 m_in_range_black;
    const __m512i m_min_brightness;
    const __m512i m_threshold;
    __m512i m_count;
};
size_t to_blackwhite_rgb32_brightness_x64_AVX512VNNI(
    const uint32_t* in, size_t in_bytes_per_row, size_t width, size_t height,
    uint32_t* out, size_t out_bytes_per_row,
    bool in_range_black,
    uint32_t min_brightness, uint32_t max_brightness
){
    ToBlackWhite_RgbBrightness_x64_AVX512 filter(in_range_black, min_brightness, max_brightness);
    filter_per_pixel(in, in_bytes_per_row, width, height, filter, out, out_bytes_per_row);
    return filter.count();
}





















}
}
#endif
