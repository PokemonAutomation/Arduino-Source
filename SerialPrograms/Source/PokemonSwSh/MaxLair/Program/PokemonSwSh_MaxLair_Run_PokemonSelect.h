/*  Max Lair Run Pokemon Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_PokemonSelect_H

#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options.h"
#include "PokemonSwSh/MaxLair/Options/PokemonSwSh_MaxLair_Options_Consoles.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_select_pokemon(
    ConsoleHandle& console, BotBaseContext& context,
    GlobalStateTracker& state_tracker,
    const ConsoleSpecificOptions& settings
);



}
}
}
}
#endif
