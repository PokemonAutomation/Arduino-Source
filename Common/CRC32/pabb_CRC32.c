/*  CRC32
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#if 0
#elif _M_IX86 || _M_X64
#include "pabb_CRC32_x86_SSE4.1.h"
#elif __AVR__
#include "pabb_CRC32_AVR8.c"
#else
#include "pabb_CRC32_Basic.c"
#endif
