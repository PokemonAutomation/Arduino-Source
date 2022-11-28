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

enum class KeyboardLayout{
    QWERTY,
    AZERTY
};

void enter_alphanumeric_code(
    Logger& logger,
    BotBaseContext& context,
    const std::string& code,
    KeyboardLayout keyboard_layout,
    bool fast
);




}
}
#endif
