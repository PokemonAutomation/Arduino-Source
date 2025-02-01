/*  Battles
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_Battles_H
#define PokemonAutomation_PokemonSV_Battles_H

#include <stdint.h>
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{




void auto_heal_from_menu_or_overworld(
    const ProgramInfo& info, VideoStream& stream, SwitchControllerContext& context,
    uint8_t party_slot, //  0 - 5
    bool return_to_overworld
);



//  Returns the # of attempts it took to run.
int run_from_battle(const ProgramInfo& info, VideoStream& stream, SwitchControllerContext& context);



}
}
}
#endif
