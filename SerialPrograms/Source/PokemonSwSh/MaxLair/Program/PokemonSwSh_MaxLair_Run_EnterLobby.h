/*  Max Lair Enter Lobby
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_EnterLobby_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_EnterLobby_H

#include "CommonFramework/Tools/ConsoleHandle.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


QImage enter_lobby(
    ConsoleHandle& console, BotBaseContext& context,
    size_t boss_slot, bool connect_to_internet,
    ReadableQuantity999& ore
);



}
}
}
}
#endif
