/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_SSE41.h"

namespace PokemonAutomation{
namespace Kernels{



struct PartialWordMask{
    size_t left;
    PartialWordAccess_x64_SSE41 loader;

    PA_FORCE_INLINE PartialWordMask(size_t p_left)
        : left(p_left)
        , loader(left * sizeof(uint32_t))
    {}
};





template <typename PixelTester>
class FilterImage_Rgb32_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    FilterImage_Rgb32_x64_SSE42(
        const PixelTester& tester,
        uint32_t replacement, bool replace_color_within_range
    )
        : m_tester(tester)
        , m_replacement(_mm_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? _mm_set1_epi32(-1) : _mm_setzero_si128())
        , m_count(_mm_setzero_si128())
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce32_x64_SSE41(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        __m128i in_range_pixels = process_word(pixel);
        m_count = _mm_sub_epi32(m_count, in_range_pixels);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m128i vmask = _mm_cmpgt_epi32(
            _mm_set1_epi32((uint32_t)mask.left),
            _mm_setr_epi32(0, 1, 2, 3)
        );

        __m128i pixel = mask.loader.load(in);
        __m128i in_range_pixels = process_word(pixel);
        in_range_pixels = _mm_and_si128(in_range_pixels, vmask);
        m_count = _mm_sub_epi32(m_count, in_range_pixels);
        size_t left = mask.left;
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    //  Process the pixel in-place.
    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m128i process_word(__m128i& pixel) const{
        __m128i mask = m_tester.test_word(pixel);
        pixel = _mm_blendv_epi8(
            m_replacement,
            pixel,
            _mm_xor_si128(mask, m_invert)
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m128i m_replacement;
    const __m128i m_invert;
    __m128i m_count;
};







template <typename PixelTester>
class ToBlackWhite_Rgb32_x64_SSE42{
public:
    static const size_t VECTOR_SIZE = 4;
    using Mask = PartialWordMask;

public:
    ToBlackWhite_Rgb32_x64_SSE42(
        const PixelTester& tester,
        bool in_range_black
    )
        : m_tester(tester)
        , m_in_range_black(_mm_set1_epi32(in_range_black ? -1 : 0))
        , m_count(_mm_setzero_si128())
    {}

    PA_FORCE_INLINE size_t count() const{
        return reduce32_x64_SSE41(m_count);
    }

    PA_FORCE_INLINE void process_full(uint32_t* out, const uint32_t* in){
        __m128i pixel = _mm_loadu_si128((const __m128i*)in);
        __m128i in_range_pixels = process_word(pixel);
        m_count = _mm_sub_epi32(m_count, in_range_pixels);
        _mm_storeu_si128((__m128i*)out, pixel);
    }
    PA_FORCE_INLINE void process_partial(uint32_t* out, const uint32_t* in, const Mask& mask){
        __m128i vmask = _mm_cmpgt_epi32(
            _mm_set1_epi32((uint32_t)mask.left),
            _mm_setr_epi32(0, 1, 2, 3)
        );

        __m128i pixel = mask.loader.load(in);
        __m128i in_range_pixels = process_word(pixel);
        in_range_pixels = _mm_and_si128(in_range_pixels, vmask);
        m_count = _mm_sub_epi32(m_count, in_range_pixels);
        size_t left = mask.left;
        do{
            out[0] = _mm_cvtsi128_si32(pixel);
            pixel = _mm_srli_si128(pixel, 4);
            out++;
        }while(--left);
    }

private:
    //  Process the pixel in-place.
    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m128i process_word(__m128i& pixel) const{
        __m128i mask = m_tester.test_word(pixel);
        pixel = _mm_or_si128(
            _mm_xor_si128(mask, m_in_range_black),
            _mm_set1_epi32(0xff000000)
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m128i m_in_range_black;
    __m128i m_count;
};




}
}
