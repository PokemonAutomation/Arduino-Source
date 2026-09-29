/*  Menu Navigation
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_MenuNavigation_H
#define PokemonAutomation_PokemonLZA_MenuNavigation_H

namespace PokemonAutomation{

template <typename Type> class ControllerContext;

namespace NintendoSwitch{

class ConsoleHandle;
class ProController;
using ProControllerContext = ControllerContext<ProController>;

namespace PokemonLZA{

// From overworld, press X to enter main menu
void overworld_to_main_menu(
    ConsoleHandle& console, ProControllerContext& context
);

// From overworld, press X to enter main menu, then A to enter box system
void overworld_to_box_system(
    ConsoleHandle& console, ProControllerContext& context
);

// From box system, press B until overworld is reached
void box_system_to_overworld(
    ConsoleHandle& console, ProControllerContext& context
);



}
}
}
#endif
