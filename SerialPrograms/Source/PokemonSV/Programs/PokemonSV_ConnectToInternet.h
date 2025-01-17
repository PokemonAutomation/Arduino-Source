/*  Connect to Integer
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ConnectToInternet_H
#define PokemonAutomation_PokemonSV_ConnectToInternet_H

namespace PokemonAutomation{
    class BotBaseContext;
    struct ProgramInfo;
namespace NintendoSwitch{
    class ConsoleHandle;
namespace PokemonSV{


//  Connect to internet from the main menu. Stay in the main menu.
void connect_to_internet_from_menu(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);

//  Connect to internet from the overworld. Return to the overworld.
void connect_to_internet_from_overworld(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);



}
}
}
#endif
