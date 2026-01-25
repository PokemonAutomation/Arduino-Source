/*  PABotBase2 Connection (Debug)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionDebug_H
#define PokemonAutomation_PABotBase2_ConnectionDebug_H

#include "PABotBase2_Connection.h"
#include "PABotBase2_PacketSender.h"
#include "PABotBase2_StreamCoalescer.h"

#ifdef __cplusplus
extern "C" {
#endif


void pabb2_PacketHeader_print(const pabb2_PacketHeader* packet, bool ascii);
void pabb2_PacketSender_print(const pabb2_PacketSender* self, bool ascii);
void pabb2_StreamCoalescer_print(const pabb2_StreamCoalescer* self, bool ascii);



#ifdef __cplusplus
}
#endif
#endif
