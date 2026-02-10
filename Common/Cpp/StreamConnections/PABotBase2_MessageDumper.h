/*  PABotBase2 Message Dumper
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PABotBase2_MessageDumper_H
#define PokemonAutomation_PABotBase2_MessageDumper_H

#include <string>
#include "Common/PABotBase2/PABotBase2_Connection.h"

namespace PokemonAutomation{


std::string tostr(const pabb2_PacketHeader* header);


}
#endif
