/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Environment_H
#define PokemonAutomation_Environment_H

#include <stdint.h>
#include <string>
#include <vector>
#include <QString>

namespace PokemonAutomation{



extern const int DEFAULT_PRIORITY_INDEX;
extern const std::vector<QString> PRIORITY_MODES;
int priority_name_to_index(const QString& name);
bool set_priority_by_index(int index);
bool set_priority_by_name(const QString& name);
int read_priority_index();





uint64_t x86_rdtsc();

void x86_cpuid(uint32_t eabcdx[4], uint32_t eax, uint32_t ecx = 0);

std::string get_processor_name();

uint64_t x86_measure_rdtsc_ticks_per_sec();
uint64_t x86_rdtsc_ticks_per_sec();


struct ProcessorSpecs{
    std::string name;
    size_t threads = 0;
    size_t cores = 0;
    size_t sockets = 0;
    size_t numa_nodes = 0;
    size_t base_frequency = 0;
};
ProcessorSpecs get_processor_specs();


struct CPU_x86_Features{
    CPU_x86_Features();

    //  Misc.
    bool HW_MMX;
    bool HW_x64;
    bool HW_ABM;
    bool HW_RDRAND;
    bool HW_RDSEED;
    bool HW_BMI1;
    bool HW_BMI2;
    bool HW_ADX;
    bool HW_MPX;
    bool HW_PREFETCHW;
    bool HW_PREFETCHWT1;
    bool HW_RDPID;

    //  SIMD: 128-bit
    bool HW_SSE;
    bool HW_SSE2;
    bool HW_SSE3;
    bool HW_SSSE3;
    bool HW_SSE41;
    bool HW_SSE42;
    bool HW_SSE4a;
    bool HW_AES;
    bool HW_SHA;

    //  SIMD: 256-bit
    bool HW_AVX;
    bool HW_XOP;
    bool HW_FMA3;
    bool HW_FMA4;
    bool HW_AVX2;

    //  SIMD: 512-bit
    bool HW_AVX512_F;
    bool HW_AVX512_CD;

    //  Knights Landing
    bool HW_AVX512_PF;
    bool HW_AVX512_ER;

    //  Skylake Purley
    bool HW_AVX512_VL;
    bool HW_AVX512_BW;
    bool HW_AVX512_DQ;

    //  Cannon Lake
    bool HW_AVX512_IFMA;
    bool HW_AVX512_VBMI;

    //  Knights Mill
    bool HW_AVX512_VPOPCNTDQ;
    bool HW_AVX512_4FMAPS;
    bool HW_AVX512_4VNNIW;

    //  Cascade Lake
    bool HW_AVX512_VNNI;

    //  Cooper Lake
    bool HW_AVX512_BF16;

    //  Ice Lake
    bool HW_AVX512_VBMI2;
    bool HW_GFNI;
    bool HW_VAES;
    bool HW_AVX512_VPCLMUL;
    bool HW_AVX512_BITALG;
};


}
#endif
