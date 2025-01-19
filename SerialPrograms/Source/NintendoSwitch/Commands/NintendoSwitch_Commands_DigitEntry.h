/*  Digit Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_DigitEntry_H
#define PokemonAutomation_NintendoSwitch_Commands_DigitEntry_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void enter_digits_str   (SwitchControllerContext& context, uint8_t count, const char* digits);
void enter_digits       (SwitchControllerContext& context, uint8_t count, const uint8_t* digits);




}
}
#endif
