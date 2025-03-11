/*  Max Lair Run Item Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H
#define PokemonAutomation_PokemonSwSh_MaxLair_Run_ItemSelect_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"
#include "PokemonSwSh/MaxLair/Framework/PokemonSwSh_MaxLair_StateTracker.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
namespace MaxLairInternal{


void run_item_select(
    size_t console_index,
    VideoStream& stream, ProControllerContext& context,
    GlobalStateTracker& state_tracker
);



}
}
}
}
#endif
