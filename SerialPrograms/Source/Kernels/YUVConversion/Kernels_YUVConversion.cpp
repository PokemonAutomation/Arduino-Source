/*  YUV Conversion
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <cmath>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Kernels_YUVConversion.h"

namespace PokemonAutomation{
namespace Kernels{


YUVCoefficients yuv_coefficients(YUVColorSpace color_space, YUVColorRange range){
    const double Kr = color_space == YUVColorSpace::BT709 ? 0.2126 : 0.299;
    const double Kb = color_space == YUVColorSpace::BT709 ? 0.0722 : 0.114;
    const double Kg = 1.0 - Kr - Kb;

    const bool limited = range == YUVColorRange::Limited;
    const double luma_scale   = limited ? 255.0 / 219.0 : 1.0;
    const double chroma_scale = limited ? 255.0 / 224.0 : 1.0;

    auto q6 = [](double value) -> int16_t{
        return (int16_t)std::lround(value * 64.0);
    };

    YUVCoefficients ret;
    ret.y_offset = limited ? 16 : 0;
    ret.y_mult   = q6(luma_scale);
    ret.cr_r     = q6(2 * (1 - Kr) * chroma_scale);
    ret.cb_b     = q6(2 * (1 - Kb) * chroma_scale);
    ret.cr_g     = q6(2 * (1 - Kr) * Kr / Kg * chroma_scale);
    ret.cb_g     = q6(2 * (1 - Kb) * Kb / Kg * chroma_scale);
    return ret;
}



#define PA_DECLARE_YUV_KERNELS(suffix) \
    void convert_YUY2_to_RGB32_##suffix( \
        size_t width, size_t height, \
        const uint8_t* src, size_t src_bytes_per_row, \
        uint32_t* dst, size_t dst_bytes_per_row, \
        const YUVCoefficients& coefficients \
    ); \
    void convert_UYVY_to_RGB32_##suffix( \
        size_t width, size_t height, \
        const uint8_t* src, size_t src_bytes_per_row, \
        uint32_t* dst, size_t dst_bytes_per_row, \
        const YUVCoefficients& coefficients \
    ); \
    void convert_YUV420P_to_RGB32_##suffix( \
        size_t width, size_t height, \
        const uint8_t* plane_Y, size_t Y_bytes_per_row, \
        const uint8_t* plane_U, size_t U_bytes_per_row, \
        const uint8_t* plane_V, size_t V_bytes_per_row, \
        uint32_t* dst, size_t dst_bytes_per_row, \
        const YUVCoefficients& coefficients \
    ); \
    void convert_NV12_to_RGB32_##suffix( \
        size_t width, size_t height, \
        const uint8_t* plane_Y, size_t Y_bytes_per_row, \
        const uint8_t* plane_UV, size_t UV_bytes_per_row, \
        bool swap_chroma, \
        uint32_t* dst, size_t dst_bytes_per_row, \
        const YUVCoefficients& coefficients \
    );

PA_DECLARE_YUV_KERNELS(Default)
PA_DECLARE_YUV_KERNELS(x64_SSE41)
PA_DECLARE_YUV_KERNELS(x64_AVX2)

#undef PA_DECLARE_YUV_KERNELS


void convert_YUY2_to_RGB32(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
#if defined(PA_AutoDispatch_x64_13_Haswell)
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_YUY2_to_RGB32_x64_AVX2(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
        return;
    }
#endif
#if defined(PA_AutoDispatch_x64_08_Nehalem)
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_YUY2_to_RGB32_x64_SSE41(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
        return;
    }
#endif
    convert_YUY2_to_RGB32_Default(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}

void convert_UYVY_to_RGB32(
    size_t width, size_t height,
    const uint8_t* src, size_t src_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
#if defined(PA_AutoDispatch_x64_13_Haswell)
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_UYVY_to_RGB32_x64_AVX2(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
        return;
    }
#endif
#if defined(PA_AutoDispatch_x64_08_Nehalem)
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_UYVY_to_RGB32_x64_SSE41(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
        return;
    }
#endif
    convert_UYVY_to_RGB32_Default(width, height, src, src_bytes_per_row, dst, dst_bytes_per_row, coefficients);
}

void convert_YUV420P_to_RGB32(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_U, size_t U_bytes_per_row,
    const uint8_t* plane_V, size_t V_bytes_per_row,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
#if defined(PA_AutoDispatch_x64_13_Haswell)
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_YUV420P_to_RGB32_x64_AVX2(
            width, height, plane_Y, Y_bytes_per_row, plane_U, U_bytes_per_row,
            plane_V, V_bytes_per_row, dst, dst_bytes_per_row, coefficients
        );
        return;
    }
#endif
#if defined(PA_AutoDispatch_x64_08_Nehalem)
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_YUV420P_to_RGB32_x64_SSE41(
            width, height, plane_Y, Y_bytes_per_row, plane_U, U_bytes_per_row,
            plane_V, V_bytes_per_row, dst, dst_bytes_per_row, coefficients
        );
        return;
    }
#endif
    convert_YUV420P_to_RGB32_Default(
        width, height, plane_Y, Y_bytes_per_row, plane_U, U_bytes_per_row,
        plane_V, V_bytes_per_row, dst, dst_bytes_per_row, coefficients
    );
}

void convert_NV12_to_RGB32(
    size_t width, size_t height,
    const uint8_t* plane_Y, size_t Y_bytes_per_row,
    const uint8_t* plane_UV, size_t UV_bytes_per_row,
    bool swap_chroma,
    uint32_t* dst, size_t dst_bytes_per_row,
    const YUVCoefficients& coefficients
){
#if defined(PA_AutoDispatch_x64_13_Haswell)
    if (CPU_CAPABILITY_CURRENT.OK_13_Haswell){
        convert_NV12_to_RGB32_x64_AVX2(
            width, height, plane_Y, Y_bytes_per_row, plane_UV, UV_bytes_per_row,
            swap_chroma, dst, dst_bytes_per_row, coefficients
        );
        return;
    }
#endif
#if defined(PA_AutoDispatch_x64_08_Nehalem)
    if (CPU_CAPABILITY_CURRENT.OK_08_Nehalem){
        convert_NV12_to_RGB32_x64_SSE41(
            width, height, plane_Y, Y_bytes_per_row, plane_UV, UV_bytes_per_row,
            swap_chroma, dst, dst_bytes_per_row, coefficients
        );
        return;
    }
#endif
    convert_NV12_to_RGB32_Default(
        width, height, plane_Y, Y_bytes_per_row, plane_UV, UV_bytes_per_row,
        swap_chroma, dst, dst_bytes_per_row, coefficients
    );
}



}
}
