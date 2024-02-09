/*  Blueberry Quests
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_BlueberryQuests_H
#define PokemonAutomation_PokemonSV_BlueberryQuests_H

namespace PokemonAutomation{
    class BotBaseContext;
    class ConsoleHandle;
    struct ProgramInfo;
namespace NintendoSwitch{
namespace PokemonSV{

// Return to Central Plaza from anywhere in the map.
void return_to_plaza(const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context);


}
}
}
#endif
