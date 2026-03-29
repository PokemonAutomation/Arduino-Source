/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageDumper_H
#define PokemonAutomation_PABotBase2_MessageDumper_H

#include <string>
#include "ReliableConnectionLayer/PABotBase2_PacketProtocol.h"
#include "PABotBase2_MessageProtocol.h"

namespace PokemonAutomation{
namespace PABotBase2{


std::string tostr(const PacketHeader* header);
std::string tostr(const MessageHeader* header);


}
}
#endif
