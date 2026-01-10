/*  CRC32
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Common_CRC32_H
#define PokemonAutomation_Common_CRC32_H


#if 0
#elif _M_IX86 || _M_X64
#include "pabb_CRC32_x86_SSE4.1.h"
#elif __AVR__
#include "pabb_CRC32_AVR8.h"
#else
#include "pabb_CRC32_Basic.h"
#endif


#endif
