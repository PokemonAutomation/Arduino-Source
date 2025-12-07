/*  Date Spamming Routines
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLGPE_Commands_DateSpam_H
#define PokemonAutomation_PokemonLGPE_Commands_DateSpam_H

#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Joycon.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

// Verify that "Date and Time" menu item is selected (not "Time Zone") before rolling date
// If wrong menu item is selected, navigate to correct it
void verify_date_time_menu_selected(ConsoleHandle& console, JoyconContext& context);

// Reset the sync clock state by toggling it ON then OFF
// This helps reset the menu state and prevents accumulated navigation errors
void reset_sync_clock_state(ConsoleHandle& console, JoyconContext& context);

void roll_date_forward_1                    (JoyconContext& context);
void roll_date_backward_N                   (JoyconContext& context, uint8_t skips);

}

}

}
#endif
