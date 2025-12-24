/*  CPU ID (x86)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_CpuId_x86_H
#define PokemonAutomation_CpuId_x86_H

#include <stdint.h>

namespace PokemonAutomation{

void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx);


struct CPU_Features{
    CPU_Features& set_to_current();

    //  CPU Compatibility
    bool OK_08_Nehalem  = false;
    bool OK_13_Haswell  = false;
    bool OK_17_Skylake  = false;
    bool OK_19_IceLake  = false;

    void update_CPU_compatibility();


    //  OS Features
    bool OS_AVX         = false;
    bool OS_AVX512      = false;

    //  Misc.
    bool HW_MMX         = false;
    bool HW_x64         = false;
    bool HW_ABM         = false;
    bool HW_RDRAND      = false;
    bool HW_RDSEED      = false;
    bool HW_BMI1        = false;
    bool HW_BMI2        = false;
    bool HW_ADX         = false;
    bool HW_MPX         = false;
    bool HW_PREFETCHW   = false;
    bool HW_PREFETCHWT1 = false;
    bool HW_RDPID       = false;

    //  SIMD: 128-bit
    bool HW_SSE         = false;
    bool HW_SSE2        = false;
    bool HW_SSE3        = false;
    bool HW_SSSE3       = false;
    bool HW_SSE41       = false;
    bool HW_SSE42       = false;
    bool HW_SSE4a       = false;
    bool HW_AES         = false;
    bool HW_SHA         = false;

    //  SIMD: 256-bit
    bool HW_AVX         = false;
    bool HW_XOP         = false;
    bool HW_FMA3        = false;
    bool HW_FMA4        = false;
    bool HW_AVX2        = false;

    //  SIMD: 512-bit
    bool HW_AVX512_F        = false;
    bool HW_AVX512_CD       = false;

    //  Knights Landing
    bool HW_AVX512_PF       = false;
    bool HW_AVX512_ER       = false;

    //  Skylake Purley
    bool HW_AVX512_VL       = false;
    bool HW_AVX512_BW       = false;
    bool HW_AVX512_DQ       = false;

    //  Cannon Lake
    bool HW_AVX512_IFMA     = false;
    bool HW_AVX512_VBMI     = false;

    //  Knights Mill
    bool HW_AVX512_VPOPCNTDQ    = false;
    bool HW_AVX512_4FMAPS       = false;
    bool HW_AVX512_4VNNIW       = false;

    //  Cascade Lake
    bool HW_AVX512_VNNI         = false;

    //  Cooper Lake
    bool HW_AVX512_BF16         = false;

    //  Ice Lake
    bool HW_AVX512_VBMI2        = false;
    bool HW_GFNI                = false;
    bool HW_VAES                = false;
    bool HW_AVX512_VPCLMUL      = false;
    bool HW_AVX512_BITALG       = false;

};


extern const CPU_Features& CPU_CAPABILITY_NOTHING();
extern const CPU_Features& CPU_CAPABILITY_09_NEHALEM();
extern const CPU_Features& CPU_CAPABILITY_13_Haswell();
extern const CPU_Features& CPU_CAPABILITY_17_Skylake();
extern const CPU_Features& CPU_CAPABILITY_19_IceLake();







}
#endif
