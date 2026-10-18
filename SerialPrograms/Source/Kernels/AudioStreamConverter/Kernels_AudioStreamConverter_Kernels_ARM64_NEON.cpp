/*  Audio Stream Converter (ARM64 NEON)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifdef PA_AutoDispatch_arm64_20_M1

#include <arm_neon.h>
#include "Common/Cpp/TestRunners/UnitTestDatabase.h"
//#include "Kernels/Kernels_arm64_NEON.h"
#include "Kernels_AudioStreamConverter_TestInfra.h"
#include "Kernels_AudioStreamConverter_Kernels_Default.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace Kernels{
namespace AudioStreamConverter{



PA_FORCE_INLINE float32x4_t convert_from_u8(uint32x4_t x, float multiplier){
    const float32x4_t SCALE0 = vdupq_n_f32(multiplier * (1.f / 128.f));
    const float32x4_t SCALE1 = vdupq_n_f32(multiplier * 1.f);
    return vsubq_f32(
        vmulq_f32(vcvtq_f32_u32(x), SCALE0),
        SCALE1
    );
}
PA_FORCE_INLINE int32x4_t convert_to_u8(float32x4_t x, float multiplier){
    const float32x4_t SCALE = vdupq_n_f32(multiplier * 128.f);
    x = vminq_f32(x, vdupq_n_f32(127.0f / 128.0f));
    x = vmaxq_f32(x, vdupq_n_f32(-1.0f));
    x = vaddq_f32(vmulq_f32(x, SCALE), SCALE);
    return vcvtq_s32_f32(x);
}
PA_FORCE_INLINE float32x4_t convert_from_s16(int32x4_t x, float multiplier){
    const float32x4_t SCALE = vdupq_n_f32(multiplier * (1.0f / 32768.0f));
    return vmulq_f32(vcvtq_f32_s32(x), SCALE);
}
PA_FORCE_INLINE int16x4_t convert_to_s16(float32x4_t x, float multiplier){
    const float32x4_t SCALE = vdupq_n_f32(multiplier * 32768.f);
    x = vmulq_f32(x, SCALE);
    x = vminq_f32(x, vdupq_n_f32(32767.f));
    x = vmaxq_f32(x, vdupq_n_f32(-32768.f));
    return vqmovn_s32(vcvtq_s32_f32(x));
}
PA_FORCE_INLINE float32x4_t convert_from_s32(int32x4_t x, float multiplier){
    const float32x4_t SCALE = vdupq_n_f32(multiplier * (1.0f / 2147483648.0f));
    return vmulq_f32(vcvtq_f32_s32(x), SCALE);
}
PA_FORCE_INLINE int32x4_t convert_to_s32(float32x4_t x, float multiplier){
    const float32x4_t SCALE = vdupq_n_f32(multiplier * 2147483648.f);
    x = vmulq_f32(x, SCALE);
    x = vminq_f32(x, vdupq_n_f32(2147483520.f)); // 2^31 - 128
    x = vmaxq_f32(x, vdupq_n_f32(-2147483648.f));
    return vcvtq_s32_f32(x);
}



template <typename ExternalType>
void convert_from_ARM64_NEON(r_ptr<float> native, c_ptr<ExternalType> ext, size_t samples, float multiplier);
template <typename ExternalType>
void convert_to_ARM64_NEON(c_ptr<float> native, r_ptr<ExternalType> ext, size_t samples, float multiplier);



template <>
void convert_from_ARM64_NEON<uint8_t>(r_ptr<float> native, c_ptr<uint8_t> ext, size_t samples, float multiplier){
    while (samples >= 8){
        uint8x8_t x = vld1_u8(ext);
        uint16x8_t x16 = vmovl_u8(x);
        uint32x4_t x32_0 = vmovl_u16(vget_low_u16(x16));
        uint32x4_t x32_1 = vmovl_u16(vget_high_u16(x16));
        vst1q_f32(native + 0, convert_from_u8(x32_0, multiplier));
        vst1q_f32(native + 4, convert_from_u8(x32_1, multiplier));

        native += 8;
        ext += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_to_ARM64_NEON<uint8_t>(c_ptr<float> native, r_ptr<uint8_t> ext, size_t samples, float multiplier){
    while (samples >= 8){
        int32x4_t x0 = convert_to_u8(vld1q_f32(native + 0), multiplier);
        int32x4_t x1 = convert_to_u8(vld1q_f32(native + 4), multiplier);
        //  Pack the low byte of each lane.
        uint16x4_t h0 = vmovn_u32(vreinterpretq_u32_s32(x0));
        uint16x4_t h1 = vmovn_u32(vreinterpretq_u32_s32(x1));
        uint8x8_t bytes = vmovn_u16(vcombine_u16(h0, h1));
        vst1_u8(ext, bytes);

        native += 8;
        ext += 8;
        samples -= 8;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_from_ARM64_NEON<int16_t>(r_ptr<float> native, c_ptr<int16_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        int16x4_t x = vld1_s16(ext);
        int32x4_t x32 = vmovl_s16(x);
        vst1q_f32(native, convert_from_s16(x32, multiplier));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_to_ARM64_NEON<int16_t>(c_ptr<float> native, r_ptr<int16_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        int16x4_t x = convert_to_s16(vld1q_f32(native), multiplier);
        vst1_s16(ext, x);

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_from_ARM64_NEON<int32_t>(r_ptr<float> native, c_ptr<int32_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        int32x4_t x = vld1q_s32(ext);
        vst1q_f32(native, convert_from_s32(x, multiplier));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_from_Default(native, ext, samples, multiplier);
    }
}
template <>
void convert_to_ARM64_NEON<int32_t>(c_ptr<float> native, r_ptr<int32_t> ext, size_t samples, float multiplier){
    while (samples >= 4){
        float32x4_t x = vld1q_f32(native);
        vst1q_s32(ext, convert_to_s32(x, multiplier));

        native += 4;
        ext += 4;
        samples -= 4;
    }
    if (samples > 0){
        convert_to_Default(native, ext, samples, multiplier);
    }
}




template <typename Type>
class Test_from_ARM64_NEON : public Test<Type>{
public:
    Test_from_ARM64_NEON(size_t samples, float multiplier)
        : Test<Type>(
            "from_ARM64_NEON",
            samples, multiplier,
            (ConvertFrom<Type>)convert_from_ARM64_NEON<Type>,
            (ConvertTo<Type>)convert_to_Default<Type>
        )
    {}
};
template <typename Type>
class Test_to_ARM64_NEON : public Test<Type>{
public:
    Test_to_ARM64_NEON(size_t samples, float multiplier)
        : Test<Type>(
            "to_ARM64_NEON",
            samples, multiplier,
            (ConvertFrom<Type>)convert_from_Default<Type>,
            (ConvertTo<Type>)convert_to_ARM64_NEON<Type>
        )
    {}
};
void add_tests_ARM64_NEON(UnitTestDatabase& database){
    for (size_t samples = 2; samples < 1000; samples = (size_t)(samples * 1.1) + 1){
        for (float multiplier = 0.1f; multiplier <= 1.0f; multiplier += 0.1f){
            database.add<Test_from_ARM64_NEON<uint8_t>>(samples, multiplier);
            database.add<Test_to_ARM64_NEON<uint8_t>>(samples, multiplier);

            database.add<Test_from_ARM64_NEON<int16_t>>(samples, multiplier);
            database.add<Test_to_ARM64_NEON<int16_t>>(samples, multiplier);

            database.add<Test_from_ARM64_NEON<int32_t>>(samples, multiplier);
            database.add<Test_to_ARM64_NEON<int32_t>>(samples, multiplier);
        }
    }
}




}
}
}
#endif
