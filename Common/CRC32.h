/*  CRC32
 * 
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 * 
 */

#ifndef PokemonAutomation_CRC32_H
#define PokemonAutomation_CRC32_H

#include <stddef.h>
#include <stdint.h>


//  Basic Implementation
uint32_t pabb_crc32_basic(uint32_t crc, const char* str, size_t length);

//  Table Implementation
uint32_t pabb_crc32_table(uint32_t crc, const char* str, size_t length);

//  SSE4.2
#if _M_IX86 || _M_X64
#include <nmmintrin.h>
static inline uint32_t pabb_crc32_SSE42(uint32_t crc, const char* str, size_t length){
    for (size_t c = 0; c < length; c++){
        crc = _mm_crc32_u8(crc, str[c]);
    }
    return crc;
}
#endif


#if _M_IX86 || _M_X64
#define pabb_crc32      pabb_crc32_SSE42
#elif __AVR__
#define pabb_crc32      pabb_crc32_table
#else
#define pabb_crc32      pabb_crc32_basic
#endif

void pabb_crc32_write_to_message(const char* ptr, size_t full_message_length);


#endif
