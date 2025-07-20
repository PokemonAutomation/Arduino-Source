/*  Serial Port (PABotBase) Post-Connect Actinos
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Controllers_SerialPABotBase_PostConnectActions_H
#define PokemonAutomation_Controllers_SerialPABotBase_PostConnectActions_H

#include "Controllers/ControllerConnection.h"

namespace PokemonAutomation{
    class PABotBase;
namespace SerialPABotBase{


void run_post_connect_actions(
    ControllerModeStatus& status,
    uint32_t program_id, const std::string& device_name,
    PABotBase& botbase,
    std::optional<ControllerType> change_controller
);




}
}
#endif
