/*  Fast Code Entry
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_FastCodeEntry_H
#define PokemonAutomation_NintendoSwitch_FastCodeEntry_H

#include <stdint.h>
#include <string>
#include <vector>

namespace PokemonAutomation{
    class Logger;
    class BotBaseContext;
namespace NintendoSwitch{



//  Internals for testing only.

struct CodeboardPosition{
    uint8_t row;
    uint8_t col;
};
struct CodeboardScroll{
    uint8_t direction : 4;
    uint8_t delay : 4;
};
static_assert(sizeof(CodeboardScroll) == sizeof(uint8_t));
struct DigitPath{
    uint8_t length = 0;
    bool left_cursor = false;
    CodeboardScroll path[14];
};
DigitPath get_codeboard_digit_path(CodeboardPosition source, CodeboardPosition destination);
void move_codeboard(BotBaseContext& context, const DigitPath& path, bool fast);



//  This is the one you want to call.

enum class KeyboardLayout{
    QWERTY,
    AZERTY
};
void enter_alphanumeric_code(
    Logger& logger,
    BotBaseContext& context,
    const std::string& code,
    KeyboardLayout keyboard_layout,
    bool include_plus, bool fast
);




}
}
#endif
