/*  Audio Stream Conversion (x86 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <immintrin.h>
#include <smmintrin.h>
#include "AudioStreamConversion.h"

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConversion{



void convert_audio_uint8_to_float_x86_SSE41(float* f, const uint8_t* i, size_t length, float output_multiplier){
    const __m128 SCALE = _mm_set1_ps(output_multiplier / 127.f);
    const __m128 SUB = _mm_set1_ps(output_multiplier);
    size_t lc = length / 4;
    while (lc--){
#if __GNUC__
        __m128i i0 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*(int32_t*)i));
#else
        __m128i i0 = _mm_cvtepu8_epi32(_mm_loadu_si32(i));
#endif
        __m128 f0 = _mm_cvtepi32_ps(i0);
        f0 = _mm_mul_ps(f0, SCALE);
        f0 = _mm_sub_ps(f0, SUB);
        f0 = _mm_max_ps(f0, _mm_set1_ps(-1.0f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(1.0f));
        _mm_storeu_ps(f, f0);
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_cvtsi32_ss(_mm_setzero_ps(), i[0]);
        f0 = _mm_mul_ss(f0, SCALE);
        f0 = _mm_sub_ss(f0, SUB);
        f0 = _mm_max_ss(f0, _mm_set1_ps(-1.0f));
        f0 = _mm_min_ss(f0, _mm_set1_ps(1.0f));
        _mm_store_ss(f, f0);
        f += 1;
        i += 1;
    }
}
void convert_audio_float_to_uint8_x86_SSE41(uint8_t* i, const float* f, size_t length){
    size_t lc = length / 4;
    while (lc--){
        __m128 f0 = _mm_loadu_ps(f);
        f0 = _mm_add_ps(f0, _mm_set1_ps(1.0f));
        f0 = _mm_mul_ps(f0, _mm_set1_ps(127.f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(255.f));
        f0 = _mm_max_ps(f0, _mm_set1_ps(0.f));
        __m128i i0 = _mm_cvtps_epi32(f0);
        i0 = _mm_shuffle_epi8(i0, _mm_setr_epi8(0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
#if __GNUC__ && __GNUC__ < 9
        *(int32_t*)i = _mm_cvtsi128_si32(i0);
#else
        _mm_storeu_si32((__m128i*)i, i0);
#endif
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_load_ss(f);
        f0 = _mm_add_ss(f0, _mm_set1_ps(1.0f));
        f0 = _mm_mul_ss(f0, _mm_set1_ps(127.f));
        f0 = _mm_min_ss(f0, _mm_set1_ps(255.f));
        f0 = _mm_max_ss(f0, _mm_set1_ps(0.f));
        i[0] = (uint8_t)_mm_cvt_ss2si(f0);
        f += 1;
        i += 1;
    }
}

void convert_audio_sint16_to_float_x86_SSE41(float* f, const int16_t* i, size_t length, float output_multiplier){
    const __m128 SCALE = _mm_set1_ps(output_multiplier / 32767.f);
    size_t lc = length / 4;
    while (lc--){
        __m128i i0 = _mm_cvtepi16_epi32(_mm_loadl_epi64((const __m128i*)i));
        __m128 f0 = _mm_cvtepi32_ps(i0);
        f0 = _mm_mul_ps(f0, SCALE);
        f0 = _mm_max_ps(f0, _mm_set1_ps(-1.0f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(1.0f));
        _mm_storeu_ps(f, f0);
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_cvtsi32_ss(_mm_setzero_ps(), i[0]);
        f0 = _mm_mul_ss(f0, SCALE);
        f0 = _mm_max_ss(f0, _mm_set1_ps(-1.0f));
        f0 = _mm_min_ss(f0, _mm_set1_ps(1.0f));
        _mm_store_ss(f, f0);
        f += 1;
        i += 1;
    }
}
void convert_audio_float_to_sint16_x86_SSE41(int16_t* i, const float* f, size_t length){
    size_t lc = length / 4;
    while (lc--){
        __m128 f0 = _mm_loadu_ps(f);
        f0 = _mm_mul_ps(f0, _mm_set1_ps(32767.f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(32767.f));
        f0 = _mm_max_ps(f0, _mm_set1_ps(-32768.f));
        __m128i i0 = _mm_cvtps_epi32(f0);
        i0 = _mm_shuffle_epi8(i0, _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, -1, -1, -1, -1, -1, -1, -1, -1));
        _mm_storel_epi64((__m128i*)i, i0);
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_load_ss(f);
        f0 = _mm_mul_ss(f0, _mm_set1_ps(32767.f));
        f0 = _mm_min_ss(f0, _mm_set1_ps(32767.f));
        f0 = _mm_max_ss(f0, _mm_set1_ps(-32768.f));
        i[0] = (int16_t)_mm_cvt_ss2si(f0);
        f += 1;
        i += 1;
    }
}

void convert_audio_sint32_to_float_x86_SSE2(float* f, const int32_t* i, size_t length, float output_multiplier){
    const __m128 SCALE = _mm_set1_ps(output_multiplier / 2147483647.f);
    size_t lc = length / 4;
    while (lc--){
        __m128i i0 = _mm_loadu_si128((const __m128i*)i);
        __m128 f0 = _mm_cvtepi32_ps(i0);
        f0 = _mm_mul_ps(f0, SCALE);
        f0 = _mm_max_ps(f0, _mm_set1_ps(-1.0f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(1.0f));
        _mm_storeu_ps(f, f0);
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_cvtsi32_ss(_mm_setzero_ps(), i[0]);
        f0 = _mm_mul_ss(f0, SCALE);
        f0 = _mm_min_ss(f0, _mm_set1_ps(32767.f));
        f0 = _mm_max_ss(f0, _mm_set1_ps(-32768.f));
        _mm_store_ss(f, f0);
        f += 1;
        i += 1;
    }
}
void convert_audio_float_to_sint32_x86_SSE2(int32_t* i, const float* f, size_t length){
    size_t lc = length / 4;
    while (lc--){
        __m128 f0 = _mm_loadu_ps(f);
        f0 = _mm_mul_ps(f0, _mm_set1_ps(2147483647.f));
        f0 = _mm_min_ps(f0, _mm_set1_ps(2147483520.f)); //  2^31 - 2^6
        f0 = _mm_max_ps(f0, _mm_set1_ps(-2147483648.f));
        __m128i i0 = _mm_cvtps_epi32(f0);
        _mm_storeu_si128((__m128i*)i, i0);
        f += 4;
        i += 4;
    }

    length %= 4;
    while (length--){
        __m128 f0 = _mm_load_ss(f);
        f0 = _mm_mul_ss(f0, _mm_set1_ps(2147483647.f));
        f0 = _mm_min_ss(f0, _mm_set1_ps(2147483520.f)); //  2^31 - 2^6
        f0 = _mm_max_ss(f0, _mm_set1_ps(-2147483648.f));
        i[0] = _mm_cvt_ss2si(f0);
        f += 1;
        i += 1;
    }
}




}
}
}
#endif
