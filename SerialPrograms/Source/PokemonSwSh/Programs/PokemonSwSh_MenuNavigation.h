/*  Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_MenuNavigation_H
#define PokemonAutomation_PokemonSwSh_MenuNavigation_H

#include <stddef.h>

namespace PokemonAutomation{
    class ConsoleHandle;
    class BotBaseContext;
namespace NintendoSwitch{
namespace PokemonSwSh{

// When Rotom Phone menu is opened, move the cursor to the target app.
// The target app index is from 0 to 9, in the order of top to bottom, left to right.
// e.g. by default, Pokemon app is at index 1, while Town Map app is at index 5.
// The function detects the current cursor location. So the function works on any initial cursor location.
void navigate_to_menu_app(ConsoleHandle& console, BotBaseContext& context, size_t app_index);


}
}
}
#endif
