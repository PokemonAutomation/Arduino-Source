/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_Commands_DateSpam_H
#define PokemonAutomation_PokemonLGPE_Commands_DateSpam_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


constexpr uint8_t MAX_YEAR = 60;


namespace PokemonLGPE{

void neutral_date_skip                      (JoyconContext& context);
void roll_date_forward_1                    (JoyconContext& context, bool fast);
void roll_date_backward_N                   (JoyconContext& context, uint8_t skips, bool fast);
void touch_date_from_home                   (JoyconContext& context, Milliseconds settings_to_home_delay);
void rollback_hours_from_home(
    JoyconContext& context,
    uint8_t hours,
    Milliseconds settings_to_home_delay
);

}

}
}
#endif
