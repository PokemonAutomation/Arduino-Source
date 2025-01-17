/*  Save Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SaveGame_H
#define PokemonAutomation_PokemonSV_SaveGame_H

#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    struct ProgramInfo;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonSV{


//  Save game from menu.
void save_game_from_menu(const ProgramInfo& info, ConsoleHandle& console, ControllerContext& context);

//  Save game from overworld.
void save_game_from_overworld(const ProgramInfo& info, ConsoleHandle& console, ControllerContext& context);

//  Save game from either menu or overworld.
void save_game_from_menu_or_overworld(
    const ProgramInfo& info, ConsoleHandle& console, ControllerContext& context,
    bool return_to_overworld
);

// Save game from overworld in the tutorial, where the minimap isn't available.
void save_game_tutorial(const ProgramInfo& info, ConsoleHandle& console, ControllerContext& context);

}
}
}
#endif
