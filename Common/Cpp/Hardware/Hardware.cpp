/*  Hardware
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Hardware.h"

#if _M_IX86 || _M_X64 || __i386__ || __x86_64__
#include "Hardware_x86.tpp"
#endif

#if _WIN32
#ifdef PA_ARCH_x86
#include "Hardware_x86_Windows.tpp"
#endif
#endif

#if defined(__linux) || defined(__APPLE__)
#ifdef PA_ARCH_x86
#include "Hardware_x86_Linux.tpp"
#elif PA_ARCH_arm64
#include "Hardware_arm64_Linux.tpp"
#endif
#endif


namespace PokemonAutomation{






}
