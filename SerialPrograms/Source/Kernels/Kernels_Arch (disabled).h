/*  Architecture
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Kernels_Arch_H
#define PokemonAutomation_Kernels_Arch_H

//  Manual ISA Override
//#define PA_Arch_x64_AVX512GF
//#define PA_Arch_x64_AVX512
//#define PA_Arch_x64_AVX2
#define PA_Arch_x64_SSE42


//  Environment implied ISAs.
#if (defined _WIN64) || (defined __x86_64)
#define PA_Arch_x64
#endif



//  Implied ISAs.

#ifdef PA_Arch_x64_AVX512GF
#define PA_Arch_x64_AVX512
#endif

#ifdef PA_Arch_x64_AVX512
#define PA_Arch_x64_AVX2
#endif

#ifdef PA_Arch_x64_AVX2
#define PA_Arch_x64_SSE42
#endif

#ifdef PA_Arch_x64_SSE42
#define PA_Arch_x64_SSE41
#endif

#ifdef PA_Arch_x64_SSE41
#define PA_Arch_x64
#endif




namespace PokemonAutomation{
namespace Kernels{




}
}


#endif
