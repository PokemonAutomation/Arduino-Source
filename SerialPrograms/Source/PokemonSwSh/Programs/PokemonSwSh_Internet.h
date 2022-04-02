/*  Internet
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_Internet_H
#define PokemonAutomation_PokemonSwSh_Internet_H

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


bool connect_to_internet_with_inference(
    ProgramEnvironment& env, BotBaseContext& context, ConsoleHandle& console,
    std::chrono::milliseconds post_wait_time = std::chrono::seconds(3),
    uint16_t timeout_ticks = 120 * TICKS_PER_SECOND
);



}
}
}
#endif
