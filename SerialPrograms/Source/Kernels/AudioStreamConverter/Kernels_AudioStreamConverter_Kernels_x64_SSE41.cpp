/*  Audio Stream Converter Kernels (x64 SSE4.1)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_x64_08_Nehalem

#include <immintrin.h>
#include <smmintrin.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
//#include "Kernels/Kernels_x64_SSE41.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioStreamConverter_Kernels_Default.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



PA_FORCE_INLINE __m128 convert_from_u8(__m128i x){
    return _mm_add_ps(
        _mm_mul_ps(
            _mm_cvtepi32_ps(x),
            _mm_set1_ps(1.f / 128.f)
        ),
        _mm_set1_ps(-1.f)
    );
}
PA_FORCE_INLINE __m128i convert_to_u8(__m128 x, float multiplier){
    const __m128 SCALE = _mm_set1_ps(multiplier * 128.f);
    x = _mm_min_ps(x, _mm_set1_ps(127.0f / 128.0f));
    x = _mm_max_ps(x, _mm_set1_ps(-1.0f));
    x = _mm_add_ps(_mm_mul_ps(x, SCALE), SCALE);
    return _mm_cvtps_epi32(x);
}
PA_FORCE_INLINE __m128 convert_from_s16(__m128i x){
    return _mm_mul_ps(
        _mm_cvtepi32_ps(x),
        _mm_set1_ps(1.0f / 32768.0f)
    );
}
PA_FORCE_INLINE __m128i convert_to_s16(__m128 x, float multiplier){
    x = _mm_mul_ps(x, _mm_set1_ps(multiplier * 32768.f));
    x = _mm_min_ps(x, _mm_set1_ps(32767.f));
    x = _mm_max_ps(x, _mm_set1_ps(-32768.f));
    return _mm_cvtps_epi32(x);
}
PA_FORCE_INLINE __m128 convert_from_s32(__m128i x){
    return _mm_mul_ps(
        _mm_cvtepi32_ps(x),
        _mm_set1_ps(1.0f / 2147483648.0f)
    );
}
PA_FORCE_INLINE __m128i convert_to_s32(__m128 x, float multiplier){
    x = _mm_mul_ps(x, _mm_set1_ps(multiplier * 2147483648.f));
    x = _mm_min_ps(x, _mm_set1_ps(2147483520.f)); // 2^31 - 128
    x = _mm_max_ps(x, _mm_set1_ps(-2147483648.f));
    return _mm_cvtps_epi32(x);
}



template <typename ExternalType>
void convert_from_x64_SSE41(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples);
template <typename ExternalType>
void convert_to_x64_SSE41(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);



template <>
void convert_from_x64_SSE41<uint8_t>(r_ptr<float> native, c_ptr<uint8_t> ext, size_t samples){
    while (samples >= 4){
        __m128i x = _mm_cvtepu8_epi32(
            _mm_loadl_epi64((const __m128i*)ext)
        );
        _mm_storeu_ps(native, convert_from_u8(x));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples);
    }
}
template <>
void convert_to_x64_SSE41<uint8_t>(c_ptr<float> native, r_ptr<uint8_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        __m128i x = convert_to_u8(_mm_loadu_ps(native), multiplier);
        x = _mm_shuffle_epi8(x, _mm_setr_epi8(
            0, 4, 8, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
        ));
        _mm_storeu_si32((__m128i*)ext, x);

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_from_x64_SSE41<int16_t>(r_ptr<float> native, c_ptr<int16_t> ext, size_t samples){
    while (samples >= 4){
        __m128i x = _mm_cvtepi16_epi32(
            _mm_loadu_si128((const __m128i*)ext)
        );
        _mm_storeu_ps(native, convert_from_s16(x));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples);
    }
}
template <>
void convert_to_x64_SSE41<int16_t>(c_ptr<float> native, r_ptr<int16_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        __m128i x = convert_to_s16(_mm_loadu_ps(native), multiplier);
        x = _mm_shuffle_epi8(x, _mm_setr_epi8(
            0, 1, 4, 5, 8, 9, 12, 13, 0, 0, 0, 0, 0, 0, 0, 0
        ));
        _mm_storeu_si64((__m128i*)ext, x);

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_from_x64_SSE41<int32_t>(r_ptr<float> native, c_ptr<int32_t> ext, size_t samples){
    while (samples >= 4){
        __m128i x = _mm_loadu_si128((const __m128i*)ext);
        _mm_storeu_ps(native, convert_from_s32(x));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples);
    }
}
template <>
void convert_to_x64_SSE41<int32_t>(c_ptr<float> native, r_ptr<int32_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        __m128 x = _mm_loadu_ps(native);
        _mm_storeu_si128((__m128i*)ext, convert_to_s32(x, multiplier));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}







template <typename Type>
class Test_from_x64_SSE41 : public Test<Type>{
public:
    Test_from_x64_SSE41(size_t samples)
        : Test<Type>(
            "from_x64_SSE41",
            samples,
            static_cast<ConvertFrom<Type>>(&convert_from_x64_SSE41<Type>),
            static_cast<ConvertTo<Type>>(&convert_to_Default<Type>)
        )
    {}
};
template <typename Type>
class Test_to_x64_SSE41 : public Test<Type>{
public:
    Test_to_x64_SSE41(size_t samples)
        : Test<Type>(
            "to_x64_SSE41",
            samples,
            static_cast<ConvertFrom<Type>>(&convert_from_Default<Type>),
            static_cast<ConvertTo<Type>>(&convert_to_x64_SSE41<Type>)
        )
    {}
};
void add_tests_x64_SSE41(UnitTestDatabase& database){
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        database.add<Test_from_x64_SSE41<uint8_t>>(samples);
        database.add<Test_to_x64_SSE41<uint8_t>>(samples);

        database.add<Test_from_x64_SSE41<int16_t>>(samples);
        database.add<Test_to_x64_SSE41<int16_t>>(samples);

        database.add<Test_from_x64_SSE41<int32_t>>(samples);
        database.add<Test_to_x64_SSE41<int32_t>>(samples);
    }
}





}
}
}
#endif
