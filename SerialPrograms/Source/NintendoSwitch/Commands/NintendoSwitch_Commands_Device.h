/*  Device Functions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Device_H
#define PokemonAutomation_NintendoSwitch_Commands_Device_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


uint32_t system_clock   (SwitchControllerContext& context);
void set_leds           (SwitchControllerContext& context, bool on);



}
}
#endif
