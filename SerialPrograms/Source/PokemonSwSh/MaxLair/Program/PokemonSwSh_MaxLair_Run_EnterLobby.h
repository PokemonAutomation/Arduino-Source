/*  Max Lair Enter Lobby
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_EnterLobby_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_EnterLobby_H

#include <memory>
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateMachine.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


std::shared_ptr<const ImageRGB32> enter_lobby(
    const ProgramInfo& info,
    VideoStream& stream, ProControllerContext& context,
    size_t boss_slot, bool connect_to_internet,
    ReadableQuantity999& ore
);



}
}
}
}
#endif
