/*  Audio Stream Converter Kernels (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioStreamConverter_Kernels_Default.h"
#include "Kernels_AudioStreamConverter_Kernels_x64_AVX512.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



__m512 convert_from_u8(__m512i x){
    return _mm512_fmadd_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(1.f / 128.f),
        _mm512_set1_ps(-1.f)
    );
}
__m512i convert_to_u8(__m512 x){
    const __m512 SCALE = _mm512_set1_ps(128.f);
    x = _mm512_min_ps(x, _mm512_set1_ps(127.0f / 128.0f));
    x = _mm512_max_ps(x, _mm512_set1_ps(-1.0f));
    x = _mm512_fmadd_ps(x, SCALE, SCALE);
    return _mm512_cvtps_epi32(x);
}
__m512 convert_from_s16(__m512i x){
    return _mm512_mul_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(1.0f / 32768.0f)
    );
}
__m512i convert_to_s16(__m512 x){
    x = _mm512_mul_ps(x, _mm512_set1_ps(32768.f));
    x = _mm512_min_ps(x, _mm512_set1_ps(32767.f));
    x = _mm512_max_ps(x, _mm512_set1_ps(-32768.f));
    return _mm512_cvtps_epi32(x);
}
__m512 convert_from_s32(__m512i x){
    return _mm512_mul_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(1.0f / 2147483648.0f)
    );
}
__m512i convert_to_s32(__m512 x){
    x = _mm512_mul_ps(x, _mm512_set1_ps(2147483648.f));
    x = _mm512_min_ps(x, _mm512_set1_ps(2147483520.f)); // 2^31 - 128
    x = _mm512_max_ps(x, _mm512_set1_ps(-2147483648.f));
    return _mm512_cvtps_epi32(x);
}



template <>
void convert_from_x64_AVX512<uint8_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const uint8_t* interleaved_stream
){
    while (samples > 16){
        __m512i x = _mm512_cvtepu8_epi32(
            _mm_loadu_si128((const __m128i*)interleaved_stream)
        );
        _mm512_storeu_ps(strided_stream, convert_from_u8(x));

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i i = _mm512_cvtepu8_epi32(
            _mm_maskz_loadu_epi8(mask, (const __m128i*)interleaved_stream)
        );
        _mm512_mask_storeu_ps(strided_stream, mask, convert_from_u8(i));
    }
}
template <>
void convert_to_x64_AVX512<uint8_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    uint8_t* interleaved_stream
){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(strided_stream);
        _mm_storeu_si128(
            (__m128i*)interleaved_stream,
            _mm512_cvtepi32_epi8(convert_to_u8(x))
        );

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, strided_stream);
        _mm_mask_storeu_epi8(
            (__m128i*)interleaved_stream, mask,
            _mm512_cvtepi32_epi8(convert_to_u8(x))
        );
    }
}
template <>
void convert_from_x64_AVX512<int16_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int16_t* interleaved_stream
){
    while (samples > 16){
        __m512i x = _mm512_cvtepi16_epi32(
            _mm256_loadu_si256((const __m256i*)interleaved_stream)
        );
        _mm512_storeu_ps(strided_stream, convert_from_s16(x));

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i x = _mm512_cvtepi16_epi32(
            _mm256_maskz_loadu_epi16(mask, interleaved_stream)
        );
        _mm512_mask_storeu_ps(strided_stream, mask, convert_from_s16(x));
    }
}
template <>
void convert_to_x64_AVX512<int16_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int16_t* interleaved_stream
){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(strided_stream);
        _mm256_storeu_si256(
            (__m256i*)interleaved_stream,
            _mm512_cvtepi32_epi16(convert_to_s16(x))
        );

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, strided_stream);
        _mm256_mask_storeu_epi16(
            (__m256i*)interleaved_stream, mask,
            _mm512_cvtepi32_epi16(convert_to_s16(x))
        );
    }
}
template <>
void convert_from_x64_AVX512<int32_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int32_t* interleaved_stream
){
    while (samples > 16){
        __m512i x = _mm512_loadu_si512(interleaved_stream);
        _mm512_storeu_ps(strided_stream, convert_from_s32(x));

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i x = _mm512_maskz_loadu_epi32(mask, interleaved_stream);
        _mm512_mask_storeu_ps(strided_stream, mask, convert_from_s32(x));
    }
}
template <>
void convert_to_x64_AVX512<int32_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int32_t* interleaved_stream
){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(strided_stream);
        _mm512_storeu_si512(interleaved_stream, convert_to_s32(x));

        strided_stream += 16;
        interleaved_stream += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, strided_stream);
        _mm512_mask_storeu_epi32(interleaved_stream, mask, convert_to_s32(x));
    }
}
template <>
void convert_from_x64_AVX512<float, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const float* interleaved_stream
){
    memcpy(strided_stream, interleaved_stream, samples * sizeof(float));
}
template <>
void convert_to_x64_AVX512<float, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    float* interleaved_stream
){
    memcpy(interleaved_stream, strided_stream, samples * sizeof(float));
}



template <>
void convert_from_x64_AVX512<uint8_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const uint8_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0;
        __m512i s0, s1;
        r0 = _mm512_cvtepu8_epi16(_mm256_loadu_si256((const __m256i*)interleaved_stream));
        s0 = _mm512_maskz_mov_epi16(0x55555555, r0);
        s1 = _mm512_srli_epi32(r0, 16);
        _mm512_storeu_ps(strided_stream + 0*stride, convert_from_u8(s0));
        _mm512_storeu_ps(strided_stream + 1*stride, convert_from_u8(s1));

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i r0;
        __m512i s0, s1;
        r0 = _mm512_cvtepu8_epi16(_mm256_maskz_loadu_epi16(mask, interleaved_stream));
        s0 = _mm512_maskz_mov_epi16(0x55555555, r0);
        s1 = _mm512_srli_epi32(r0, 16);
        _mm512_mask_storeu_ps(strided_stream + 0*stride, mask, convert_from_u8(s0));
        _mm512_mask_storeu_ps(strided_stream + 1*stride, mask, convert_from_u8(s1));
    }
}
template <>
void convert_to_x64_AVX512<uint8_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    uint8_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0, r1;
        __m256i s0;
        r0 = convert_to_u8(_mm512_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_u8(_mm512_loadu_ps(strided_stream + 1*stride));
        r0 = _mm512_permutex2var_epi16(
            r0,
            _mm512_setr_epi16(
                 0, 32,  2, 34,  4, 36,  6, 38,
                 8, 40, 10, 42, 12, 44, 14, 46,
                16, 48, 18, 50, 20, 52, 22, 54,
                24, 56, 26, 58, 28, 60, 30, 62
            ),
            r1
        );
        s0 = _mm512_cvtepi16_epi8(r0);
        _mm256_storeu_si256((__m256i*)interleaved_stream, s0);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i r0, r1;
        __m256i s0;
        r0 = convert_to_u8(_mm512_maskz_loadu_ps(mask, strided_stream + 0*stride));
        r1 = convert_to_u8(_mm512_maskz_loadu_ps(mask, strided_stream + 1*stride));
        r0 = _mm512_permutex2var_epi16(
            r0,
            _mm512_setr_epi16(
                 0, 32,  2, 34,  4, 36,  6, 38,
                 8, 40, 10, 42, 12, 44, 14, 46,
                16, 48, 18, 50, 20, 52, 22, 54,
                24, 56, 26, 58, 28, 60, 30, 62
            ),
            r1
        );
        s0 = _mm512_cvtepi16_epi8(r0);
        _mm256_mask_storeu_epi16(interleaved_stream, mask, s0);
    }
}
template <>
void convert_from_x64_AVX512<int16_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int16_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0;
        __m512i s0, s1;
        r0 = _mm512_loadu_si512(interleaved_stream);
        s0 = _mm512_slli_epi32(r0, 16);
        s0 = _mm512_srai_epi32(s0, 16);
        s1 = _mm512_srai_epi32(r0, 16);
        _mm512_storeu_ps(strided_stream + 0*stride, convert_from_s16(s0));
        _mm512_storeu_ps(strided_stream + 1*stride, convert_from_s16(s1));

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i r0;
        __m512i s0, s1;
        r0 = _mm512_maskz_loadu_epi32(mask, interleaved_stream);
        s0 = _mm512_slli_epi32(r0, 16);
        s0 = _mm512_srai_epi32(s0, 16);
        s1 = _mm512_srai_epi32(r0, 16);
        _mm512_mask_storeu_ps(strided_stream + 0*stride, mask, convert_from_s16(s0));
        _mm512_mask_storeu_ps(strided_stream + 1*stride, mask, convert_from_s16(s1));
    }
}
template <>
void convert_to_x64_AVX512<int16_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int16_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0, r1;
        __m512i s0;
        r0 = convert_to_s16(_mm512_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_s16(_mm512_loadu_ps(strided_stream + 1*stride));
        s0 = _mm512_permutex2var_epi16(
            r0,
            _mm512_setr_epi16(
                 0, 32,  2, 34,  4, 36,  6, 38,
                 8, 40, 10, 42, 12, 44, 14, 46,
                16, 48, 18, 50, 20, 52, 22, 54,
                24, 56, 26, 58, 28, 60, 30, 62
            ),
            r1
        );
        _mm512_storeu_si512(interleaved_stream, s0);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i r0, r1;
        __m512i s0;
        r0 = convert_to_s16(_mm512_maskz_loadu_ps(mask, strided_stream + 0*stride));
        r1 = convert_to_s16(_mm512_maskz_loadu_ps(mask, strided_stream + 1*stride));
        s0 = _mm512_permutex2var_epi16(
            r0,
            _mm512_setr_epi16(
                 0, 32,  2, 34,  4, 36,  6, 38,
                 8, 40, 10, 42, 12, 44, 14, 46,
                16, 48, 18, 50, 20, 52, 22, 54,
                24, 56, 26, 58, 28, 60, 30, 62
            ),
            r1
        );
        _mm512_mask_storeu_epi32(interleaved_stream, mask, s0);
    }
}
template <>
void convert_from_x64_AVX512<int32_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int32_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0, r1;
        __m512i s0, s1;
        r0 = _mm512_loadu_si512(interleaved_stream +  0);
        r1 = _mm512_loadu_si512(interleaved_stream + 16);
        s0 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_storeu_ps(strided_stream + 0*stride, convert_from_s32(s0));
        _mm512_storeu_ps(strided_stream + 1*stride, convert_from_s32(s1));

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512i r0, r1;
        __m512i s0, s1;
        r0 = _mm512_maskz_loadu_epi32(mask0, interleaved_stream +  0);
        r1 = _mm512_maskz_loadu_epi32(mask1, interleaved_stream + 16);
        s0 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_mask_storeu_ps(strided_stream + 0*stride, mask, convert_from_s32(s0));
        _mm512_mask_storeu_ps(strided_stream + 1*stride, mask, convert_from_s32(s1));
    }
}
template <>
void convert_to_x64_AVX512<int32_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int32_t* interleaved_stream
){
    while (samples > 16){
        __m512i r0, r1;
        __m512i s0, s1;
        r0 = convert_to_s32(_mm512_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_s32(_mm512_loadu_ps(strided_stream + 1*stride));
        s0 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_storeu_si512(interleaved_stream +  0, s0);
        _mm512_storeu_si512(interleaved_stream + 16, s1);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512i r0, r1;
        __m512i s0, s1;
        r0 = convert_to_s32(_mm512_maskz_loadu_ps(mask, strided_stream + 0*stride));
        r1 = convert_to_s32(_mm512_maskz_loadu_ps(mask, strided_stream + 1*stride));
        s0 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_epi32(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_mask_storeu_epi32(interleaved_stream +  0, mask0, s0);
        _mm512_mask_storeu_epi32(interleaved_stream + 16, mask1, s1);
    }
}
template <>
void convert_from_x64_AVX512<float, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const float* interleaved_stream
){
    while (samples > 16){
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_loadu_ps(interleaved_stream +  0);
        r1 = _mm512_loadu_ps(interleaved_stream + 16);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_storeu_ps(strided_stream + 0*stride, s0);
        _mm512_storeu_ps(strided_stream + 1*stride, s1);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_maskz_loadu_ps(mask0, interleaved_stream +  0);
        r1 = _mm512_maskz_loadu_ps(mask1, interleaved_stream + 16);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32(1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31), r1);
        _mm512_mask_storeu_ps(strided_stream + 0*stride, mask, s0);
        _mm512_mask_storeu_ps(strided_stream + 1*stride, mask, s1);
    }
}
template <>
void convert_to_x64_AVX512<float, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    float* interleaved_stream
){
    while (samples > 16){
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_loadu_ps(strided_stream + 0*stride);
        r1 = _mm512_loadu_ps(strided_stream + 1*stride);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_storeu_ps(interleaved_stream +  0, s0);
        _mm512_storeu_ps(interleaved_stream + 16, s1);

        strided_stream += 16;
        interleaved_stream += 32;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        uint64_t double_mask = ((uint64_t)1 << samples*2) - 1;
        __mmask16 mask0 = (__mmask16)double_mask;
        __mmask16 mask1 = (__mmask16)(double_mask >> 16);
        __m512 r0, r1;
        __m512 s0, s1;
        r0 = _mm512_maskz_loadu_ps(mask, strided_stream + 0*stride);
        r1 = _mm512_maskz_loadu_ps(mask, strided_stream + 1*stride);
        s0 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 0, 16,  1, 17,  2, 18,  3, 19,  4, 20,  5, 21,  6, 22,  7, 23), r1);
        s1 = _mm512_permutex2var_ps(r0, _mm512_setr_epi32( 8, 24,  9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31), r1);
        _mm512_mask_storeu_ps(interleaved_stream +  0, mask0, s0);
        _mm512_mask_storeu_ps(interleaved_stream + 16, mask1, s1);
    }
}










template <typename Type>
void convert_from_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_from_x64_AVX512<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_from_x64_AVX512<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_from_Default<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}
template <typename Type>
void convert_to_x64_AVX512(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_to_x64_AVX512<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_to_x64_AVX512<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_to_Default<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}






template <typename Type>
class Test_Kernel_from_x64_AVX512 : public Test_Kernel<Type>{
public:
    Test_Kernel_from_x64_AVX512(size_t channels, size_t samples)
        : Test_Kernel<Type>(
            "from_x64_AVX512",
            channels, samples,
            convert_from_x64_AVX512<Type>,
            convert_to_Default<Type>
        )
    {}
};
template <typename Type>
class Test_Kernel_to_x64_AVX512 : public Test_Kernel<Type>{
public:
    Test_Kernel_to_x64_AVX512(size_t channels, size_t samples)
        : Test_Kernel<Type>(
            "to_x64_AVX512",
            channels, samples,
            convert_from_Default<Type>,
            convert_to_x64_AVX512<Type>
        )
    {}
};

void add_tests_x64_AVX512(UnitTestDatabase& database){
    for (size_t c = 1; c <= 3; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_Kernel_from_x64_AVX512<uint8_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX512<uint8_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX512<int16_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX512<int16_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX512<int32_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX512<int32_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX512<float>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX512<float>>(c, samples);
        }
    }
}






}
}
}
#endif
