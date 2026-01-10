/*  CRC32
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Common_CRC32_SSE41_H
#define PokemonAutomation_Common_CRC32_SSE41_H

#include <stdint.h>
#include <string.h>
#include <nmmintrin.h>

#ifdef __cplusplus
extern "C" {
#endif



static inline void pabb_crc32_buffer(uint32_t* crc, const void* data, size_t length){
    uint32_t tmp = *crc;
    const char* ptr = (const char*)data;
    for (size_t c = 0; c < length; c++){
        tmp = _mm_crc32_u8(tmp, ptr[c]);
    }
    *crc = tmp;
}

static inline void pabb_crc32_write_to_message(void* data, size_t full_message_length){
    char* ptr = (char*)data;
    size_t length_before_crc = full_message_length - sizeof(uint32_t);
    uint32_t crc = 0xffffffff;
    pabb_crc32_buffer(&crc, ptr, length_before_crc);
    memcpy(ptr + length_before_crc, &crc, sizeof(uint32_t));
}



#ifdef __cplusplus
}
#endif
#endif
