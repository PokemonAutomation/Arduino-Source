/*  Device Functions
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_Commands_Device_H
#define PokemonAutomation_NintendoSwitch_Commands_Device_H

#include "ClientSource/Connection/BotBase.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


uint32_t system_clock   (BotBaseControllerContext& context);
void set_leds           (BotBaseControllerContext& context, bool on);



}
}
#endif
