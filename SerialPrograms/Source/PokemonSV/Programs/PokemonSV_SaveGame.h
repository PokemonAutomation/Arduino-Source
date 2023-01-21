/*  Save Game
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_SaveGame_H
#define PokemonAutomation_PokemonSV_SaveGame_H

namespace PokemonAutomation{
    class BotBaseContext;
    class ConsoleHandle;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{


//  Save game from menu.
void save_game_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Save game from overworld.
void save_game_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);



}
}
}
#endif
