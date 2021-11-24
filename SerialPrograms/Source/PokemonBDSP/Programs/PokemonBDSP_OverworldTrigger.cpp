/*  Overworld Trigger
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch/Commands/NintendoSwitch_PushButtons.h"
#include "PokemonBDSP_OverworldTrigger.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


OverworldTrigger::OverworldTrigger()
    : GroupOption("Trigger Method")
    , TRIGGER_METHOD(
        "<b>Maneuver:</b><br>How to trigger an encounter",
        {
            "Move left/right. (no bias)",
            "Move left/right. (bias left)",
            "Move left/right. (bias right)",
            "Move up/down. (no bias)",
            "Move up/down. (bias up)",
            "Move up/down. (bias down)",
        }, 0
    )
    , MOVE_DURATION(
        "<b>Move Duration:</b><br>Move in each direction for this long before turning around.",
        "1 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(TRIGGER_METHOD);
    PA_ADD_OPTION(MOVE_DURATION);
}
void OverworldTrigger::run_trigger(const BotBaseContext& context) const{
    switch (TRIGGER_METHOD){
    case 0:
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case 1:
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION, 0);
        break;
    case 2:
        pbf_move_left_joystick(context, 255, 128, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 0, 128, MOVE_DURATION, 0);
        break;
    case 3:
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case 4:
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION, 0);
        break;
    case 5:
        pbf_move_left_joystick(context, 128, 255, MOVE_DURATION + 25, 0);
        pbf_move_left_joystick(context, 128, 0, MOVE_DURATION, 0);
        break;
    }
}



}
}
}
