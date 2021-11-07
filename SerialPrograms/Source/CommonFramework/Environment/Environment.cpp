/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Environment.h"

namespace PokemonAutomation{


CPU_x86_Features::CPU_x86_Features(){
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
}


}
