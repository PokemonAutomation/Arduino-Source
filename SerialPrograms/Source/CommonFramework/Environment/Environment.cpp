/*  Environment
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/CpuId/CpuId.h"
#include "Environment.h"

#if _M_IX86 || _M_X64 || __i386__ || __x86_64__
#include "Environment_x86.tpp"
#endif

#if _WIN32
#include "Environment_Windows.tpp"
#ifdef PA_ARCH_x86
#include "Environment_x86_Windows.tpp"
#endif
#endif

#if defined(__linux) || defined(__APPLE__)
#include "Environment_Linux.tpp"
#ifdef PA_ARCH_x86
#include "Environment_x86_Linux.tpp"
#elif PA_ARCH_arm64
#include "Environment_arm64_Linux.tpp"
#endif
#endif


namespace PokemonAutomation{






}
