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
namespace PokemonLGPE{

void roll_date_forward_1                    (JoyconContext& context);
void roll_date_backward_N                   (JoyconContext& context, uint8_t skips);

}

}
}
#endif
