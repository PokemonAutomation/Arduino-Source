/*  Digit Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_DigitEntry_H
#define PokemonAutomation_NintendoSwitch_Commands_DigitEntry_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


void enter_digits_str   (const BotBaseContext& context, uint8_t count, const char* digits);
void enter_digits       (const BotBaseContext& context, uint8_t count, const uint8_t* digits);




}
}
#endif
