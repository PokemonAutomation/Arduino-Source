/*  PABotBase2 Stream Interface
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_StreamInterface_H
#define PokemonAutomation_PABotBase2_StreamInterface_H

#ifdef __cplusplus
extern "C" {
#endif


typedef size_t (*pabb2_fp_StreamSend)(void* context, const void* data, size_t bytes);
typedef size_t (*pabb2_fp_StreamRecv)(void* context, void* data, size_t max_bytes);






#ifdef __cplusplus
}
#endif
#endif
