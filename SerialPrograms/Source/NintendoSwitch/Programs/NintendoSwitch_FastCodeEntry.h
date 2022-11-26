/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FastCodeEntry_H
#define PokemonAutomation_NintendoSwitch_FastCodeEntry_H

#include <stdint.h>
#include <string>

namespace PokemonAutomation{
    class Logger;
    class BotBaseContext;
namespace NintendoSwitch{



void enter_alphanumeric_code(
    Logger& logger,
    BotBaseContext& context,
    const std::string& code
);




}
}
#endif
