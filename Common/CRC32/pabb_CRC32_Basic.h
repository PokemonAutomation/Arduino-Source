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



//
//  These implement the CRC32c with polynomial 0x11EDC6F41.
//

extern const uint32_t PABB_CRC32C_TABLE8[];

void pabb_crc32c_buffer(uint32_t* crc, const void* data, size_t length);

static inline void pabb_crc32c_write_to_message(uint32_t crc, void* data, size_t full_message_length){
    char* ptr = (char*)data;
    size_t length_before_crc = full_message_length - sizeof(uint32_t);
    pabb_crc32c_buffer(&crc, ptr, length_before_crc);
    memcpy(ptr + length_before_crc, &crc, sizeof(uint32_t));
}



#ifdef __cplusplus
}
#endif
#endif
