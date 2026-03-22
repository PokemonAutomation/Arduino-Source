/*  PABotBase2 Connection (Debug)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionDebug_H
#define PokemonAutomation_PABotBase2_ConnectionLayer_ConnectionDebug_H

#include "PABotBase2_Connection.h"

namespace PokemonAutomation{
namespace PABotBase2{


void PacketHeader_print(const PacketHeader* packet, bool ascii);
bool PacketHeader_check(const PacketHeader* packet, bool ascii);



}
}
#endif
