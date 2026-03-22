/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageDumper_H
#define PokemonAutomation_PABotBase2_MessageDumper_H

#include <string>
#include "Common/PABotBase2/ConnectionLayer/PABotBase2_Connection.h"
#include "Common/PABotBase2/DataLayer/PABotBase2_MessageProtocol.h"

namespace PokemonAutomation{
namespace PABotBase2{


std::string tostr(const PacketHeader* header);
std::string tostr(const pabb2_MessageHeader* header);


}
}
#endif
