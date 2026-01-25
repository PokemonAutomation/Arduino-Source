/*  PABotBase2 Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_StreamInterface_H
#define PokemonAutomation_PABotBase2_StreamInterface_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef size_t (*pabb2_fp_StreamSend)(
    void* context,
    const void* data, size_t bytes,
    bool is_retransmit
);
typedef size_t (*pabb2_fp_StreamRecv)(
    void* context,
    void* data, size_t max_bytes
);






#ifdef __cplusplus
}
#endif
#endif
