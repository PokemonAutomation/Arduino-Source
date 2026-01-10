/*  CRC32
 * 
 *  From: https://github.com/PokemonAutomation/
 * 
 */

#ifndef PokemonAutomation_Common_CRC32_Basic_H
#define PokemonAutomation_Common_CRC32_Basic_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


extern const uint32_t PABB_CRC32_TABLE8[];


void pabb_crc32_buffer(uint32_t* crc, const void* data, uint8_t length);

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
