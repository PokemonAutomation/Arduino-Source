/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"


namespace PokemonAutomation{
namespace Kernels{



struct PartialWordMask{
    __mmask16 m;

    PA_FORCE_INLINE PartialWordMask(size_t left)
        : m(((__mmask16)1 << left) - 1)
    {}
};




template <typename PixelTester>
class FilterImage_Rgb32_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    FilterImage_Rgb32_x64_AVX512(
        const PixelTester& tester,
        uint32_t replacement, bool replace_color_within_range
    )
        : m_tester(tester)
        , m_replacement(_mm512_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? 0xffff : 0)
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
        __mmask16 mask = m_tester.test_word(pixel);
        pixel = _mm512_mask_blend_epi32(
            mask ^ m_invert,
            m_replacement,
            pixel
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m512i m_replacement;
    const __mmask16 m_invert;
    __m512i m_count;
};





template <typename PixelTester>
class ToBlackWhite_Rgb32_x64_AVX512{
public:
    static const size_t VECTOR_SIZE = 16;
    using Mask = PartialWordMask;

public:
    ToBlackWhite_Rgb32_x64_AVX512(
        const PixelTester& tester,
        bool in_range_black
    )
        : m_tester(tester)
        , m_in_range_color(_mm512_set1_epi32(in_range_black ? 0xff000000 : 0xffffffff))
        , m_out_range_color(_mm512_set1_epi32(in_range_black ? 0xffffffff : 0xff000000))
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
        __mmask16 mask = m_tester.test_word(pixel);
        pixel = _mm512_mask_blend_epi32(
            mask,
            m_out_range_color,
            m_in_range_color
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m512i m_in_range_color;
    const __m512i m_out_range_color;
    __m512i m_count;
};





}
}
