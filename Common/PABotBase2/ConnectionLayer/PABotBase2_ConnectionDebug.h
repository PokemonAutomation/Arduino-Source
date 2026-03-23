/*  PABotBase2 Connection (Debug)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionDebug_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionDebug_H

#include <cstddef>
#include "PABotBase2_Connection.h"

namespace PokemonAutomation{
namespace PABotBase2{

void print_bytes(const void* data, size_t bytes, bool ascii);
void PacketHeader_print(const PacketHeader* packet, bool ascii);
bool PacketHeader_check(const PacketHeader* packet, bool ascii);



}
}
#endif
