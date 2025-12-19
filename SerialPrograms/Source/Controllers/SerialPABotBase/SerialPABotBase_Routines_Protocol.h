/*  SerialPABotBase Routines (Protocol)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SerialPABotBase_Routines_Protocol_H
#define PokemonAutomation_SerialPABotBase_Routines_Protocol_H

#include "Common/SerialPABotBase/SerialPABotBase_Protocol_IDs.h"
#include "Controllers/SerialPABotBase/Connection/BotBase.h"

namespace PokemonAutomation{
namespace SerialPABotBase{

uint32_t protocol_version(BotBaseController& botbase);
uint32_t program_version(BotBaseController& botbase);
uint8_t program_id(BotBaseController& botbase);
std::string program_name(BotBaseController& botbase);
std::vector<pabb_ControllerID> controller_list(BotBaseController& botbase);
uint8_t device_queue_size(BotBaseController& botbase);
uint32_t read_controller_mode(BotBaseController& botbase);
uint32_t change_controller_mode(BotBaseController& botbase, uint32_t mode);



}
}
#endif
