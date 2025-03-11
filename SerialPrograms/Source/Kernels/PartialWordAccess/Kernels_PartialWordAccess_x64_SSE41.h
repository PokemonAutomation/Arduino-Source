/*  Partial Word Access (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_PartialWordAccess_x64_SSE41_H
#define PokemonAutomation_Kernels_PartialWordAccess_x64_SSE41_H

#include <stdint.h>
#include <cstddef>
#include <smmintrin.h>
#include "Common/Compiler.h"

namespace PokemonAutomation{
namespace Kernels{


class PartialWordAccess_x64_SSE41{
public:
    PA_FORCE_INLINE PartialWordAccess_x64_SSE41(size_t bytes)
        : m_shift(16 - bytes)
    {
        m_front_mask = _mm_cmpgt_epi8(
            _mm_set1_epi8((uint8_t)bytes),
            _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15)
        );

        m_shift_front = _mm_setr_epi8(-128, -127, -126, -125, -124, -123, -122, -121, -120, -119, -118, -117, -116, -115, -114, -113);
        m_shift_front = _mm_sub_epi8(
            m_shift_front,
            _mm_set1_epi8((uint8_t)bytes)
        );

        m_shift_back = _mm_sub_epi8(
            _mm_setr_epi8(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15),
            _mm_set1_epi8((uint8_t)m_shift)
        );
    }

    PA_FORCE_INLINE __m128i load_int_no_read_past_end(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        return _mm_shuffle_epi8(x, m_shift_front);
    }
    PA_FORCE_INLINE __m128i load_int_no_read_before_ptr(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)ptr);
        return _mm_and_si128(x, m_front_mask);
    }
    PA_FORCE_INLINE __m128 load_f32_no_read_past_end(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        return _mm_castsi128_ps(_mm_shuffle_epi8(x, m_shift_front));
    }
    PA_FORCE_INLINE __m128 load_f32_no_read_before_ptr(const void* ptr) const{
        __m128i x = _mm_loadu_si128((const __m128i*)ptr);
        return _mm_castsi128_ps(_mm_and_si128(x, m_front_mask));
    }
    PA_FORCE_INLINE __m128i load(const void* ptr) const{
        const void* end = (const char*)ptr + 16;
        if (((size_t)end & 4095) < 16){
            return load_int_no_read_past_end(ptr);
        }else{
            return load_int_no_read_before_ptr(ptr);
        }
    }

    PA_FORCE_INLINE void store_int_no_past_end(void* ptr, __m128i x) const{
        __m128i v = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        x = _mm_shuffle_epi8(x, m_shift_back);
        v = _mm_blendv_epi8(x, v, m_shift_back);
        _mm_storeu_si128((__m128i*)((char*)ptr - m_shift), v);
    }
    PA_FORCE_INLINE void store_f32_no_past_end(void* ptr, __m128 x) const{
        __m128i v = _mm_loadu_si128((const __m128i*)((const char*)ptr - m_shift));
        __m128i i = _mm_castps_si128(x);
        i = _mm_shuffle_epi8(i, m_shift_back);
        v = _mm_blendv_epi8(i, v, m_shift_back);
        _mm_storeu_si128((__m128i*)((char*)ptr - m_shift), v);
    }

private:
    size_t m_shift;
    __m128i m_front_mask;
    __m128i m_shift_front;
    __m128i m_shift_back;
};



}
}
#endif
