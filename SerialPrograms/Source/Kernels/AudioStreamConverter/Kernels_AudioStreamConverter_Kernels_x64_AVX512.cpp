/*  Audio Stream Converter (x64 AVX512)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_17_Skylake

#include <immintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
//#include "Kernels/Kernels_x64_AVX512.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioStreamConverter_Kernels_Default.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



PA_FORCE_INLINE __m512 convert_from_u8(__m512i x, float multiplier){
    return _mm512_fmadd_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(multiplier * (1.f / 128.f)),
        _mm512_set1_ps(-multiplier)
    );
}
PA_FORCE_INLINE __m512i convert_to_u8(__m512 x, float multiplier){
    //  Scale first, then clamp: a loud signal with gain clips to the
    //  uint8 range, it does not scale past it.
    x = _mm512_mul_ps(x, _mm512_set1_ps(multiplier));
    x = _mm512_min_ps(x, _mm512_set1_ps(127.0f / 128.0f));
    x = _mm512_max_ps(x, _mm512_set1_ps(-1.0f));
    x = _mm512_fmadd_ps(x, _mm512_set1_ps(128.0f), _mm512_set1_ps(128.5f));
    return _mm512_cvttps_epi32(x);
}
//  VPMOVQB saturates int32 lane values above 127, which would clamp the
//  upper half of the uint8 range.  Shift the [0, 255] values into signed
//  byte range first, then add the 128 back in byte lanes (modulo 256).
PA_FORCE_INLINE __m128i pack_u8_bytes(__m512i x){
    x = _mm512_sub_epi32(x, _mm512_set1_epi32(128));
    __m128i y = _mm512_cvtepi32_epi8(x);
    return _mm_add_epi8(y, _mm_set1_epi8(-128));
}
PA_FORCE_INLINE __m512 convert_from_s16(__m512i x, float multiplier){
    return _mm512_mul_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(multiplier * (1.0f / 32768.0f))
    );
}
PA_FORCE_INLINE __m512i convert_to_s16(__m512 x, float multiplier){
    x = _mm512_mul_ps(x, _mm512_set1_ps(multiplier * 32768.f));
    x = _mm512_min_ps(x, _mm512_set1_ps(32767.f));
    x = _mm512_max_ps(x, _mm512_set1_ps(-32768.f));
    //  Round half away from zero like the Default path: offset by
    //  +/-0.5, then truncate.
    __m512 positive = _mm512_cmp_ps(x, _mm512_setzero_ps(), _CMP_GT_OQ);
    x = _mm512_add_ps(x, _mm512_blendv_ps(_mm512_set1_ps(-0.5f), _mm512_set1_ps(0.5f), positive));
    return _mm512_cvttps_epi32(x);
}
PA_FORCE_INLINE __m512 convert_from_s32(__m512i x, float multiplier){
    return _mm512_mul_ps(
        _mm512_cvtepi32_ps(x),
        _mm512_set1_ps(multiplier * (1.0f / 2147483648.0f))
    );
}
PA_FORCE_INLINE __m512i convert_to_s32(__m512 x, float multiplier){
    x = _mm512_mul_ps(x, _mm512_set1_ps(multiplier * 2147483648.f));
    x = _mm512_min_ps(x, _mm512_set1_ps(2147483520.f)); // 2^31 - 128
    x = _mm512_max_ps(x, _mm512_set1_ps(-2147483648.f));
    //  Round half away from zero like the Default path: offset by
    //  +/-0.5, then truncate.
    __m512 positive = _mm512_cmp_ps(x, _mm512_setzero_ps(), _CMP_GT_OQ);
    x = _mm512_add_ps(x, _mm512_blendv_ps(_mm512_set1_ps(-0.5f), _mm512_set1_ps(0.5f), positive));
    return _mm512_cvttps_epi32(x);
}



template <typename ExternalType>
void convert_from_x64_AVX512(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples, float multiplier);
template <typename ExternalType>
void convert_to_x64_AVX512(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);
void add_tests_x64_AVX512(UnitTestDatabase& database);



template <>
void convert_from_x64_AVX512<uint8_t>(r_ptr<float> native, c_ptr<uint8_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512i x = _mm512_cvtepu8_epi32(
            _mm_loadu_si128((const __m128i*)ext)
        );
        _mm512_storeu_ps(native, convert_from_u8(x, multiplier));

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i i = _mm512_cvtepu8_epi32(
            _mm_maskz_loadu_epi8(mask, (const __m128i*)ext)
        );
        _mm512_mask_storeu_ps(native, mask, convert_from_u8(i, multiplier));
    }
}
template <>
void convert_to_x64_AVX512<uint8_t>(c_ptr<float> native, r_ptr<uint8_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(native);
        _mm_storeu_si128(
            (__m128i*)ext,
            pack_u8_bytes(convert_to_u8(x, multiplier))
        );

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, native);
        _mm_mask_storeu_epi8(
            (__m128i*)ext, mask,
            pack_u8_bytes(convert_to_u8(x, multiplier))
        );
    }
}
template <>
void convert_from_x64_AVX512<int16_t>(r_ptr<float> native, c_ptr<int16_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512i x = _mm512_cvtepi16_epi32(
            _mm256_loadu_si256((const __m256i*)ext)
        );
        _mm512_storeu_ps(native, convert_from_s16(x, multiplier));

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i x = _mm512_cvtepi16_epi32(
            _mm256_maskz_loadu_epi16(mask, ext)
        );
        _mm512_mask_storeu_ps(native, mask, convert_from_s16(x, multiplier));
    }
}
template <>
void convert_to_x64_AVX512<int16_t>(c_ptr<float> native, r_ptr<int16_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(native);
        _mm256_storeu_si256(
            (__m256i*)ext,
            _mm512_cvtepi32_epi16(convert_to_s16(x, multiplier))
        );

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, native);
        _mm256_mask_storeu_epi16(
            (__m256i*)ext, mask,
            _mm512_cvtepi32_epi16(convert_to_s16(x, multiplier))
        );
    }
}
template <>
void convert_from_x64_AVX512<int32_t>(r_ptr<float> native, c_ptr<int32_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512i x = _mm512_loadu_si512(ext);
        _mm512_storeu_ps(native, convert_from_s32(x, multiplier));

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512i x = _mm512_maskz_loadu_epi32(mask, ext);
        _mm512_mask_storeu_ps(native, mask, convert_from_s32(x, multiplier));
    }
}
template <>
void convert_to_x64_AVX512<int32_t>(c_ptr<float> native, r_ptr<int32_t> ext, size_t samples, float multiplier){
    while (samples > 16){
        __m512 x = _mm512_loadu_ps(native);
        _mm512_storeu_si512(ext, convert_to_s32(x, multiplier));

        native += 16;
        ext += 16;
        samples -= 16;
    }
    {
        __mmask16 mask = ((uint32_t)1 << samples) - 1;
        __m512 x = _mm512_maskz_loadu_ps(mask, native);
        _mm512_mask_storeu_epi32(ext, mask, convert_to_s32(x, multiplier));
    }
}







template <typename Type>
class Test_from_x64_AVX512 : public Test<Type>{
public:
    Test_from_x64_AVX512(size_t samples, float from_multiplier, float to_multiplier)
        : Test<Type>(
            "from_x64_AVX512",
            samples,
            (ConvertFrom<Type>)convert_from_x64_AVX512<Type>,
            (ConvertTo<Type>)convert_to_Default<Type>,
            from_multiplier,
            to_multiplier
        )
    {}
};
template <typename Type>
class Test_to_x64_AVX512 : public Test<Type>{
public:
    Test_to_x64_AVX512(size_t samples, float from_multiplier, float to_multiplier)
        : Test<Type>(
            "to_x64_AVX512",
            samples,
            (ConvertFrom<Type>)convert_from_Default<Type>,
            (ConvertTo<Type>)convert_to_x64_AVX512<Type>,
            from_multiplier,
            to_multiplier
        )
    {}
};
template <typename Type>
class Test_to_arb_x64_AVX512 : public ToArbitraryTest<Type>{
public:
    Test_to_arb_x64_AVX512(size_t samples)
        : ToArbitraryTest<Type>(
            "to_arb_x64_AVX512",
            samples,
            (ConvertTo<Type>)convert_to_x64_AVX512<Type>
        )
    {}
};

void add_tests_x64_AVX512(UnitTestDatabase& database){
    const float from_multipliers[] = { 1.0f, 0.5f, 0.25f };
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        for (float from_multiplier : from_multipliers){
            float to_multiplier = 1.0f / from_multiplier;
            database.add<Test_from_x64_AVX512<uint8_t>>(samples, from_multiplier, to_multiplier);
            database.add<Test_to_x64_AVX512<uint8_t>>(samples, from_multiplier, to_multiplier);

            database.add<Test_from_x64_AVX512<int16_t>>(samples, from_multiplier, to_multiplier);
            database.add<Test_to_x64_AVX512<int16_t>>(samples, from_multiplier, to_multiplier);

            database.add<Test_from_x64_AVX512<int32_t>>(samples, from_multiplier, to_multiplier);
            database.add<Test_to_x64_AVX512<int32_t>>(samples, from_multiplier, to_multiplier);
        }
    }
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        database.add<Test_to_arb_x64_AVX512<uint8_t>>(samples);
        database.add<Test_to_arb_x64_AVX512<int16_t>>(samples);
        database.add<Test_to_arb_x64_AVX512<int32_t>>(samples);
    }

}






}
}
}
#endif
