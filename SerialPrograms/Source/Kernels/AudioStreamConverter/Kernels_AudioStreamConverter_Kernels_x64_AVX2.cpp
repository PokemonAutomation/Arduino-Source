/*  Audio Stream Converter Kernels (x64 AVX256)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_13_Haswell

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTest.h"
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
//#include "Kernels/Kernels_x64_AVX2.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioStreamConverter_Kernels_Default.h"
#include "Kernels_AudioStreamConverter_Kernels_x64_AVX2.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



__m256 convert_from_u8(__m256i x){
    return _mm256_fmadd_ps(
        _mm256_cvtepi32_ps(x),
        _mm256_set1_ps(1.f / 128.f),
        _mm256_set1_ps(-1.f)
    );
}
__m256i convert_to_u8(__m256 x){
    const __m256 SCALE = _mm256_set1_ps(128.f);
    x = _mm256_min_ps(x, _mm256_set1_ps(127.0f / 128.0f));
    x = _mm256_max_ps(x, _mm256_set1_ps(-1.0f));
    x = _mm256_fmadd_ps(x, SCALE, SCALE);
    return _mm256_cvtps_epi32(x);
}
__m256 convert_from_s16(__m256i x){
    return _mm256_mul_ps(
        _mm256_cvtepi32_ps(x),
        _mm256_set1_ps(1.0f / 32768.0f)
    );
}
__m256i convert_to_s16(__m256 x){
    x = _mm256_mul_ps(x, _mm256_set1_ps(32768.f));
    x = _mm256_min_ps(x, _mm256_set1_ps(32767.f));
    x = _mm256_max_ps(x, _mm256_set1_ps(-32768.f));
    return _mm256_cvtps_epi32(x);
}
__m256 convert_from_s32(__m256i x){
    return _mm256_mul_ps(
        _mm256_cvtepi32_ps(x),
        _mm256_set1_ps(1.0f / 2147483648.0f)
    );
}
__m256i convert_to_s32(__m256 x){
    x = _mm256_mul_ps(x, _mm256_set1_ps(2147483648.f));
    x = _mm256_min_ps(x, _mm256_set1_ps(2147483520.f)); // 2^31 - 128
    x = _mm256_max_ps(x, _mm256_set1_ps(-2147483648.f));
    return _mm256_cvtps_epi32(x);
}



template <>
void convert_from_x64_AVX2<uint8_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const uint8_t* interleaved_stream
){
    while (samples >= 8){
        __m256i x = _mm256_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i*)interleaved_stream)
        );
        _mm256_storeu_ps(strided_stream, convert_from_u8(x));

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<uint8_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<uint8_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    uint8_t* interleaved_stream
){
    while (samples >= 8){
        __m256i x = convert_to_u8(_mm256_loadu_ps(strided_stream));
        x = _mm256_shuffle_epi8(x, _mm256_setr_epi8(
            0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        ));
        _mm_storeu_si32(
            (__m128i*)(interleaved_stream + 0),
            _mm256_castsi256_si128(x)
        );
        _mm_storeu_si32(
            (__m128i*)(interleaved_stream + 4),
            _mm256_extracti128_si256(x, 1)
        );

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<uint8_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<int16_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int16_t* interleaved_stream
){
    while (samples >= 8){
        __m256i x = _mm256_cvtepi16_epi32(
            _mm_loadu_si128((const __m128i*)interleaved_stream)
        );
        _mm256_storeu_ps(strided_stream, convert_from_s16(x));

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<int16_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<int16_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int16_t* interleaved_stream
){
    while (samples >= 8){
        __m256i x = convert_to_s16(_mm256_loadu_ps(strided_stream));
        x = _mm256_shuffle_epi8(x, _mm256_setr_epi8(
            0, 1, 4, 5, 8, 9, 12, 13, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 1, 4, 5, 8, 9, 12, 13, 0, 0, 0, 0, 0, 0, 0, 0
        ));
        _mm_storeu_si64(
            (__m128i*)(interleaved_stream + 0),
            _mm256_castsi256_si128(x)
        );
        _mm_storeu_si64(
            (__m128i*)(interleaved_stream + 4),
            _mm256_extracti128_si256(x, 1)
        );

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<int16_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<int32_t, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int32_t* interleaved_stream
){
    while (samples >= 8){
        __m256i x = _mm256_loadu_si256((const __m256i*)interleaved_stream);
        _mm256_storeu_ps(strided_stream, convert_from_s32(x));

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<int32_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<int32_t, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int32_t* interleaved_stream
){
    while (samples >= 8){
        __m256 x = _mm256_loadu_ps(strided_stream);
        _mm256_storeu_si256((__m256i*)interleaved_stream, convert_to_s32(x));

        strided_stream += 8;
        interleaved_stream += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<int32_t, 1>(samples, stride, strided_stream, interleaved_stream);
    }
}



template <>
void convert_from_x64_AVX2<uint8_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const uint8_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0;
        __m256i s0, s1;
        r0 = _mm256_cvtepu8_epi16(_mm_loadu_si128((const __m128i*)interleaved_stream));
        s0 = _mm256_and_si256(r0, _mm256_set1_epi32(0x0000ffff));
        s1 = _mm256_srli_epi32(r0, 16);
        _mm256_storeu_ps(strided_stream + 0*stride, convert_from_u8(s0));
        _mm256_storeu_ps(strided_stream + 1*stride, convert_from_u8(s1));

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<uint8_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<uint8_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    uint8_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0, r1;
        __m256i s0;
        r0 = convert_to_u8(_mm256_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_u8(_mm256_loadu_ps(strided_stream + 1*stride));
        r0 = _mm256_shuffle_epi8(r0, _mm256_setr_epi8(
            0, 15, 4, 15, 8, 15, 12, 15, 15, 15, 15, 15, 15, 15, 15, 15,
            0, 15, 4, 15, 8, 15, 12, 15, 15, 15, 15, 15, 15, 15, 15, 15
        ));
        r1 = _mm256_shuffle_epi8(r1, _mm256_setr_epi8(
            15, 0, 15, 4, 15, 8, 15, 12, 15, 15, 15, 15, 15, 15, 15, 15,
            15, 0, 15, 4, 15, 8, 15, 12, 15, 15, 15, 15, 15, 15, 15, 15
        ));
        s0 = _mm256_or_si256(r0, r1);
        s0 = _mm256_permute4x64_epi64(s0, 1*0 + 4*2 + 16*0 + 64*2);
        _mm_storeu_si128((__m128i*)interleaved_stream, _mm256_castsi256_si128(s0));

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<uint8_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<int16_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int16_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0;
        __m256i s0, s1;
        r0 = _mm256_loadu_si256((const __m256i*)interleaved_stream);
        s0 = _mm256_slli_epi32(r0, 16);
        s0 = _mm256_srai_epi32(s0, 16);
        s1 = _mm256_srai_epi32(r0, 16);
        _mm256_storeu_ps(strided_stream + 0*stride, convert_from_s16(s0));
        _mm256_storeu_ps(strided_stream + 1*stride, convert_from_s16(s1));

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<int16_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<int16_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int16_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0, r1;
        __m256i s0;
        r0 = convert_to_s16(_mm256_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_s16(_mm256_loadu_ps(strided_stream + 1*stride));
        r0 = _mm256_and_si256(r0, _mm256_set1_epi32(0x0000ffff));
        r1 = _mm256_slli_epi32(r1, 16);
        s0 = _mm256_or_si256(r0, r1);
        _mm256_storeu_si256((__m256i*)interleaved_stream, s0);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<int16_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<int32_t, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const int32_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0, r1;
        __m256i s0, s1;
        r0 = _mm256_loadu_si256((const __m256i*)interleaved_stream + 0);
        r1 = _mm256_loadu_si256((const __m256i*)interleaved_stream + 1);
        r0 = _mm256_permutevar8x32_epi32(r0, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
        r1 = _mm256_permutevar8x32_epi32(r1, _mm256_setr_epi32(1, 3, 5, 7, 0, 2, 4, 6));
        s0 = _mm256_blend_epi32(r0, r1, 0xf0);
        s1 = _mm256_permute2x128_si256(r0, r1, 33);
        _mm256_storeu_ps(strided_stream + 0*stride, convert_from_s32(s0));
        _mm256_storeu_ps(strided_stream + 1*stride, convert_from_s32(s1));

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<int32_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<int32_t, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    int32_t* interleaved_stream
){
    while (samples >= 8){
        __m256i r0, r1;
        __m256i s0, s1;
        r0 = convert_to_s32(_mm256_loadu_ps(strided_stream + 0*stride));
        r1 = convert_to_s32(_mm256_loadu_ps(strided_stream + 1*stride));
        r0 = _mm256_permutevar8x32_epi32(r0, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
        r1 = _mm256_permutevar8x32_epi32(r1, _mm256_setr_epi32(7, 0, 4, 1, 5, 2, 6, 3));
        s0 = _mm256_blend_epi32(r0, r1, 0xaa);
        s1 = _mm256_blend_epi32(r0, r1, 0x55);
        s1 = _mm256_permutevar8x32_epi32(s1, _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0));
        _mm256_storeu_si256((__m256i*)interleaved_stream + 0, s0);
        _mm256_storeu_si256((__m256i*)interleaved_stream + 1, s1);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<int32_t, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<float, 2>(
    size_t samples,
    size_t stride, float* strided_stream,
    const float* interleaved_stream
){
    while (samples >= 8){
        __m256 r0, r1;
        __m256 s0, s1;
        r0 = _mm256_loadu_ps(interleaved_stream + 0);
        r1 = _mm256_loadu_ps(interleaved_stream + 8);
        r0 = _mm256_permutevar8x32_ps(r0, _mm256_setr_epi32(0, 2, 4, 6, 1, 3, 5, 7));
        r1 = _mm256_permutevar8x32_ps(r1, _mm256_setr_epi32(1, 3, 5, 7, 0, 2, 4, 6));
        s0 = _mm256_blend_ps(r0, r1, 0xf0);
        s1 = _mm256_permute2f128_ps(r0, r1, 33);
        _mm256_storeu_ps(strided_stream + 0*stride, s0);
        _mm256_storeu_ps(strided_stream + 1*stride, s1);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default<float, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_to_x64_AVX2<float, 2>(
    size_t samples,
    size_t stride, const float* strided_stream,
    float* interleaved_stream
){
    while (samples >= 8){
        __m256 r0, r1;
        __m256 s0, s1;
        r0 = _mm256_loadu_ps(strided_stream + 0*stride);
        r1 = _mm256_loadu_ps(strided_stream + 1*stride);
        r0 = _mm256_permutevar8x32_ps(r0, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));
        r1 = _mm256_permutevar8x32_ps(r1, _mm256_setr_epi32(7, 0, 4, 1, 5, 2, 6, 3));
        s0 = _mm256_blend_ps(r0, r1, 0xaa);
        s1 = _mm256_blend_ps(r0, r1, 0x55);
        s1 = _mm256_permutevar8x32_ps(s1, _mm256_setr_epi32(1, 2, 3, 4, 5, 6, 7, 0));
        _mm256_storeu_ps(interleaved_stream + 0, s0);
        _mm256_storeu_ps(interleaved_stream + 8, s1);

        strided_stream += 8;
        interleaved_stream += 16;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default<float, 2>(samples, stride, strided_stream, interleaved_stream);
    }
}









template <typename Type>
void convert_from_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, float* strided_stream,
    const Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_from_x64_AVX2<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_from_x64_AVX2<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_from_Default<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}
template <typename Type>
void convert_to_x64_AVX2(
    size_t channels, size_t samples,
    size_t stride, const float* strided_stream,
    Type* interleaved_stream
){
    //  Special case the lower channel counts so they can be force-inlined with
    //  the inner loop unrolled.
    switch (channels){
    case 1:
        convert_to_x64_AVX2<Type, 1>(samples, stride, strided_stream, interleaved_stream);
        break;
    case 2:
        convert_to_x64_AVX2<Type, 2>(samples, stride, strided_stream, interleaved_stream);
        break;
    default:
        convert_to_Default<Type>(channels, samples, stride, strided_stream, interleaved_stream);
    }
}
template <>
void convert_from_x64_AVX2<float, 1>(
    size_t samples,
    size_t stride, float* strided_stream,
    const float* interleaved_stream
){
    memcpy(strided_stream, interleaved_stream, samples * sizeof(float));
}
template <>
void convert_to_x64_AVX2<float, 1>(
    size_t samples,
    size_t stride, const float* strided_stream,
    float* interleaved_stream
){
    memcpy(interleaved_stream, strided_stream, samples * sizeof(float));
}






template <typename Type>
class Test_Kernel_from_x64_AVX2 : public Test_Kernel<Type>{
public:
    Test_Kernel_from_x64_AVX2(size_t channels, size_t samples)
        : Test_Kernel<Type>(
            "from_x64_AVX2",
            channels, samples,
            convert_from_x64_AVX2<Type>,
            convert_to_Default<Type>
        )
    {}
};
template <typename Type>
class Test_Kernel_to_x64_AVX2 : public Test_Kernel<Type>{
public:
    Test_Kernel_to_x64_AVX2(size_t channels, size_t samples)
        : Test_Kernel<Type>(
            "to_x64_AVX2",
            channels, samples,
            convert_from_Default<Type>,
            convert_to_x64_AVX2<Type>
        )
    {}
};

void add_tests_x64_AVX2(UnitTestDatabase& database){
    for (size_t c = 1; c <= 3; c++){
        for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
            database.add<Test_Kernel_from_x64_AVX2<uint8_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX2<uint8_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX2<int16_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX2<int16_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX2<int32_t>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX2<int32_t>>(c, samples);

            database.add<Test_Kernel_from_x64_AVX2<float>>(c, samples);
            database.add<Test_Kernel_to_x64_AVX2<float>>(c, samples);
        }
    }
}





}
}
}
#endif
