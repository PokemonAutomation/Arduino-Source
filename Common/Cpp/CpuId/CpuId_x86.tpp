/*  CPU ID (x86)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <stdint.h>

#ifdef _WIN32
#include <intrin.h>
#include <Windows.h>
#endif

#if __GNUC__
#ifndef cpuid_H
#define cpuid_H
#include <cpuid.h>
#endif
#endif

#include "CpuId.h"

namespace PokemonAutomation{

#if _M_X64 || __x86_64__
const char* PA_ARCH_STRING = "x64";
#else
const char* PA_ARCH_STRING = "x86";
#endif


#if __GNUC__
void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    __cpuid_count(eax, ecx, eabcdx[0], eabcdx[1], eabcdx[2], eabcdx[3]);
}
uint64_t xgetbv(unsigned int index){
    uint32_t eax, edx;
    __asm__ volatile ("xgetbv" : "=a"(eax), "=d"(edx) : "c"(index));
    return ((uint64_t)edx << 32) | eax;
}
#define _XCR_XFEATURE_ENABLED_MASK  0
#else
void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx){
    int out[4];
    __cpuidex(out, eax, ecx);
    eabcdx[0] = out[0];
    eabcdx[1] = out[1];
    eabcdx[2] = out[2];
    eabcdx[3] = out[3];
}
__int64 xgetbv(unsigned int x){
    return _xgetbv(x);
}
#endif


bool detect_OS_AVX(){
    //  Copied from: http://stackoverflow.com/a/22521619/922184

    bool avxSupported = false;

    uint32_t cpuInfo[4];
    x86_cpuid(cpuInfo, 1, 0);

    bool osUsesXSAVE_XRSTORE = (cpuInfo[2] & (1 << 27)) != 0;
    bool cpuAVXSuport = (cpuInfo[2] & (1 << 28)) != 0;

    if (osUsesXSAVE_XRSTORE && cpuAVXSuport)
    {
        uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
        avxSupported = (xcrFeatureMask & 0x6) == 0x6;
    }

    return avxSupported;
}
bool detect_OS_AVX512(){
    if (!detect_OS_AVX())
        return false;

    uint64_t xcrFeatureMask = xgetbv(_XCR_XFEATURE_ENABLED_MASK);
    return (xcrFeatureMask & 0xe6) == 0xe6;
}


CPU_Features& CPU_Features::set_to_current(){
    //  OS Features
    OS_AVX = detect_OS_AVX();
    OS_AVX512 = detect_OS_AVX512();

    uint32_t info[4];
    x86_cpuid(info, 0, 0);
    uint32_t nIds = info[0];

    x86_cpuid(info, 0x80000000, 0);
    uint32_t nExIds = info[0];

    //  Detect Features
    if (nIds >= 0x00000001){
        x86_cpuid(info, 0x00000001, 0);
        HW_MMX    = (info[3] & ((int)1 << 23)) != 0;
        HW_SSE    = (info[3] & ((int)1 << 25)) != 0;
        HW_SSE2   = (info[3] & ((int)1 << 26)) != 0;
        HW_SSE3   = (info[2] & ((int)1 <<  0)) != 0;

        HW_SSSE3  = (info[2] & ((int)1 <<  9)) != 0;
        HW_SSE41  = (info[2] & ((int)1 << 19)) != 0;
        HW_SSE42  = (info[2] & ((int)1 << 20)) != 0;
        HW_AES    = (info[2] & ((int)1 << 25)) != 0;

        HW_AVX    = (info[2] & ((int)1 << 28)) != 0;
        HW_FMA3   = (info[2] & ((int)1 << 12)) != 0;

        HW_RDRAND = (info[2] & ((int)1 << 30)) != 0;
    }
    if (nIds >= 0x00000007){
        x86_cpuid(info, 0x00000007, 0);
        HW_AVX2         = (info[1] & ((int)1 <<  5)) != 0;

        HW_BMI1         = (info[1] & ((int)1 <<  3)) != 0;
        HW_BMI2         = (info[1] & ((int)1 <<  8)) != 0;
        HW_ADX          = (info[1] & ((int)1 << 19)) != 0;
        HW_MPX          = (info[1] & ((int)1 << 14)) != 0;
        HW_SHA          = (info[1] & ((int)1 << 29)) != 0;
        HW_RDSEED       = (info[1] & ((int)1 << 18)) != 0;
        HW_PREFETCHWT1  = (info[2] & ((int)1 <<  0)) != 0;
        HW_RDPID        = (info[2] & ((int)1 << 22)) != 0;

        HW_AVX512_F     = (info[1] & ((int)1 << 16)) != 0;
        HW_AVX512_CD    = (info[1] & ((int)1 << 28)) != 0;
        HW_AVX512_PF    = (info[1] & ((int)1 << 26)) != 0;
        HW_AVX512_ER    = (info[1] & ((int)1 << 27)) != 0;

        HW_AVX512_VL    = (info[1] & ((int)1 << 31)) != 0;
        HW_AVX512_BW    = (info[1] & ((int)1 << 30)) != 0;
        HW_AVX512_DQ    = (info[1] & ((int)1 << 17)) != 0;

        HW_AVX512_IFMA  = (info[1] & ((int)1 << 21)) != 0;
        HW_AVX512_VBMI  = (info[2] & ((int)1 <<  1)) != 0;

        HW_AVX512_VPOPCNTDQ = (info[2] & ((int)1 << 14)) != 0;
        HW_AVX512_4FMAPS    = (info[3] & ((int)1 <<  2)) != 0;
        HW_AVX512_4VNNIW    = (info[3] & ((int)1 <<  3)) != 0;

        HW_AVX512_VNNI      = (info[2] & ((int)1 << 11)) != 0;

        HW_AVX512_VBMI2     = (info[2] & ((int)1 <<  6)) != 0;
        HW_GFNI             = (info[2] & ((int)1 <<  8)) != 0;
        HW_VAES             = (info[2] & ((int)1 <<  9)) != 0;
        HW_AVX512_VPCLMUL   = (info[2] & ((int)1 << 10)) != 0;
        HW_AVX512_BITALG    = (info[2] & ((int)1 << 12)) != 0;


        x86_cpuid(info, 0x00000007, 1);
        HW_AVX512_BF16      = (info[0] & ((int)1 <<  5)) != 0;

    }
    if (nExIds >= 0x80000001){
        x86_cpuid(info, 0x80000001, 0);
        HW_x64          = (info[3] & ((int)1 << 29)) != 0;
        HW_ABM          = (info[2] & ((int)1 <<  5)) != 0;
        HW_SSE4a        = (info[2] & ((int)1 <<  6)) != 0;
        HW_PREFETCHW    = (info[2] & ((int)1 <<  8)) != 0;
        HW_XOP          = (info[2] & ((int)1 << 11)) != 0;
        HW_FMA4         = (info[2] & ((int)1 << 16)) != 0;
    }

    update_CPU_compatibility();
    return *this;
}

void CPU_Features::update_CPU_compatibility(){
    OK_08_Nehalem = true;
    OK_08_Nehalem &= HW_SSE42;

    OK_13_Haswell = OK_08_Nehalem;
    OK_13_Haswell &= OS_AVX;
    OK_13_Haswell &= HW_BMI2;
    OK_13_Haswell &= HW_FMA3;
    OK_13_Haswell &= HW_AVX2;

    OK_17_Skylake = OK_13_Haswell;
    OK_17_Skylake &= HW_AVX512_F;
    OK_17_Skylake &= HW_AVX512_CD;
    OK_17_Skylake &= HW_AVX512_VL;
    OK_17_Skylake &= HW_AVX512_BW;
    OK_17_Skylake &= HW_AVX512_DQ;

    OK_19_IceLake = OK_17_Skylake;
    OK_19_IceLake &= HW_AVX512_IFMA;
    OK_19_IceLake &= HW_AVX512_VBMI;
    OK_19_IceLake &= HW_AVX512_VPOPCNTDQ;
    OK_19_IceLake &= HW_AVX512_VNNI;
    OK_19_IceLake &= HW_AVX512_VBMI2;
    OK_19_IceLake &= HW_GFNI;
    OK_19_IceLake &= HW_VAES;
    OK_19_IceLake &= HW_AVX512_VPCLMUL;
    OK_19_IceLake &= HW_AVX512_BITALG;
}


const CPU_Features&  CPU_CAPABILITY_NATIVE(){
    static const CPU_Features ret = CPU_Features().set_to_current();
    return ret;
}



const CPU_Features& CPU_CAPABILITY_NOTHING(){
    static const CPU_Features ret;
    return ret;
}

CPU_Features make_09_Nehalem(){
    CPU_Features ret;

    ret.HW_MMX = true;
    ret.HW_x64 = true;

    ret.HW_SSE = true;
    ret.HW_SSE2 = true;
    ret.HW_SSE3 = true;
    ret.HW_SSSE3 = true;
    ret.HW_SSE41 = true;
    ret.HW_SSE42 = true;

    ret.update_CPU_compatibility();
    return ret;
}
const CPU_Features& CPU_CAPABILITY_09_NEHALEM(){
    static const CPU_Features ret = make_09_Nehalem();
    return ret;
}

CPU_Features make_13_Haswell(){
    CPU_Features ret;

    ret.OS_AVX = CPU_CAPABILITY_NATIVE().OS_AVX;

    ret.HW_MMX = true;
    ret.HW_x64 = true;
    ret.HW_ABM = true;
    ret.HW_RDRAND = true;
    ret.HW_BMI1 = true;
    ret.HW_BMI2 = true;

    ret.HW_SSE = true;
    ret.HW_SSE2 = true;
    ret.HW_SSE3 = true;
    ret.HW_SSSE3 = true;
    ret.HW_SSE41 = true;
    ret.HW_SSE42 = true;

    ret.HW_AVX = true;
    ret.HW_FMA3 = true;
    ret.HW_AVX2 = true;

    ret.update_CPU_compatibility();
    return ret;
}
const CPU_Features& CPU_CAPABILITY_13_Haswell(){
    static const CPU_Features ret = make_13_Haswell();
    return ret;
}

CPU_Features make_17_Skylake(){
    CPU_Features ret;

    ret.OS_AVX = CPU_CAPABILITY_NATIVE().OS_AVX;
    ret.OS_AVX512 = CPU_CAPABILITY_NATIVE().OS_AVX512;

    ret.HW_MMX = true;
    ret.HW_x64 = true;
    ret.HW_ABM = true;
    ret.HW_RDRAND = true;
    ret.HW_RDSEED = true;
    ret.HW_BMI1 = true;
    ret.HW_BMI2 = true;
    ret.HW_ADX = true;
    ret.HW_PREFETCHW = true;

    ret.HW_SSE = true;
    ret.HW_SSE2 = true;
    ret.HW_SSE3 = true;
    ret.HW_SSSE3 = true;
    ret.HW_SSE41 = true;
    ret.HW_SSE42 = true;
    ret.HW_AES = true;

    ret.HW_AVX = true;
    ret.HW_FMA3 = true;
    ret.HW_AVX2 = true;

    ret.HW_AVX512_F = true;
    ret.HW_AVX512_CD = true;
    ret.HW_AVX512_VL = true;
    ret.HW_AVX512_BW = true;
    ret.HW_AVX512_DQ = true;

    ret.update_CPU_compatibility();
    return ret;
}
const CPU_Features& CPU_CAPABILITY_17_Skylake(){
    static const CPU_Features ret = make_17_Skylake();
    return ret;
}

CPU_Features make_19_IceLake(){
    CPU_Features ret;

    ret.OS_AVX = CPU_CAPABILITY_NATIVE().OS_AVX;
    ret.OS_AVX512 = CPU_CAPABILITY_NATIVE().OS_AVX512;

    ret.HW_MMX = true;
    ret.HW_x64 = true;
    ret.HW_ABM = true;
    ret.HW_RDRAND = true;
    ret.HW_RDSEED = true;
    ret.HW_BMI1 = true;
    ret.HW_BMI2 = true;
    ret.HW_ADX = true;
    ret.HW_PREFETCHW = true;

    ret.HW_SSE = true;
    ret.HW_SSE2 = true;
    ret.HW_SSE3 = true;
    ret.HW_SSSE3 = true;
    ret.HW_SSE41 = true;
    ret.HW_SSE42 = true;
    ret.HW_AES = true;

    ret.HW_AVX = true;
    ret.HW_FMA3 = true;
    ret.HW_AVX2 = true;

    ret.HW_AVX512_F = true;
    ret.HW_AVX512_CD = true;
    ret.HW_AVX512_VL = true;
    ret.HW_AVX512_BW = true;
    ret.HW_AVX512_DQ = true;
    ret.HW_AVX512_IFMA = true;
    ret.HW_AVX512_VBMI = true;
    ret.HW_AVX512_VPOPCNTDQ = true;
    ret.HW_AVX512_VNNI = true;
    ret.HW_AVX512_VBMI2 = true;
    ret.HW_GFNI = true;
    ret.HW_VAES = true;
    ret.HW_AVX512_VPCLMUL = true;
    ret.HW_AVX512_BITALG = true;

    ret.update_CPU_compatibility();
    return ret;
}
const CPU_Features& CPU_CAPABILITY_19_IceLake(){
    static const CPU_Features ret = make_19_IceLake();
    return ret;
}

const std::vector<CpuCapabilityOption>& AVAILABLE_CAPABILITIES(){
    static const std::vector<CpuCapabilityOption> LIST{
        {
            "none", "Nothing (C++ Only)",
            CPU_CAPABILITY_NOTHING(), true
        },
#ifdef PA_AutoDispatch_x64_08_Nehalem
        {
            "nehalem-sse4.2", "Intel Nehalem (x64 SSE4.2)",
            CPU_CAPABILITY_09_NEHALEM(), CPU_CAPABILITY_NATIVE().OK_08_Nehalem
        },
#endif
#ifdef PA_AutoDispatch_x64_13_Haswell
        {
            "haswell-avx2", "Intel Haswell (x64 AVX2)",
            CPU_CAPABILITY_13_Haswell(), CPU_CAPABILITY_NATIVE().OK_13_Haswell
        },
#endif
#ifdef PA_AutoDispatch_x64_17_Skylake
        {
            "skylake-avx512", "Intel Skylake (x64 AVX512)",
            CPU_CAPABILITY_17_Skylake(), CPU_CAPABILITY_NATIVE().OK_17_Skylake
        },
#endif
#ifdef PA_AutoDispatch_x64_19_IceLake
        {
            "icelake-avx512gf", "Intel Ice Lake (x64 AVX512-GF)",
            CPU_CAPABILITY_19_IceLake(),  CPU_CAPABILITY_NATIVE().OK_19_IceLake
        },
#endif
    };
    return LIST;
}






}


