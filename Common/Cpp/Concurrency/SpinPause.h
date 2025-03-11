/*  Spin Pause
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SpinPause_H
#define PokemonAutomation_SpinPause_H

#include "Common/Compiler.h"


#if 0

#elif _M_IX86 || _M_X64 || __i386__ || __x86_64__
#include <emmintrin.h>
namespace PokemonAutomation{
    PA_FORCE_INLINE void pause(){
        _mm_pause();
    }
}
#else
namespace PokemonAutomation{
    PA_FORCE_INLINE void pause(){}
}
#endif


#endif
