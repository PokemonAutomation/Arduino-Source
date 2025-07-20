/*  Image Filters Basic Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>
#include <immintrin.h>
#include "Common/Compiler.h"
#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels/PartialWordAccess/Kernels_PartialWordAccess_x64_AVX2.h"

namespace PokemonAutomation{
namespace Kernels{



template <typename PixelTester>
class FilterImage_Rgb32_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    FilterImage_Rgb32_x64_AVX2(
        const PixelTester& tester,
        uint32_t replacement, bool replace_color_within_range
    )
        : m_tester(tester)
        , m_replacement(_mm256_set1_epi32(replacement))
        , m_invert(replace_color_within_range ? _mm256_set1_epi32(-1) : _mm256_setzero_si256())
        , m_count(_mm256_setzero_si256())
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
    //  Process the pixel in-place.
    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m256i process_word(__m256i& pixel) const{
        __m256i mask = m_tester.test_word(pixel);
        pixel = _mm256_blendv_epi8(
            m_replacement,
            pixel,
            _mm256_xor_si256(mask, m_invert)
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m256i m_replacement;
    const __m256i m_invert;
    __m256i m_count;
};




template <typename PixelTester>
class ToBlackWhite_Rgb32_x64_AVX2{
public:
    static const size_t VECTOR_SIZE = 8;
    using Mask = PartialWordAccess32_x64_AVX2;

public:
    ToBlackWhite_Rgb32_x64_AVX2(
        const PixelTester& tester,
        bool in_range_black
    )
        : m_tester(tester)
        , m_in_range_black(_mm256_set1_epi32(in_range_black ? -1 : 0))
        , m_count(_mm256_setzero_si256())
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
    //  Process the pixel in-place.
    //  Return a mask indicating which lanes are in range.
    PA_FORCE_INLINE __m256i process_word(__m256i& pixel) const{
        __m256i mask = m_tester.test_word(pixel);
        pixel = _mm256_or_si256(
            _mm256_xor_si256(mask, m_in_range_black),
            _mm256_set1_epi32(0xff000000)
        );
        return mask;
    }

private:
    const PixelTester m_tester;
    const __m256i m_in_range_black;
    __m256i m_count;
};




}
}
