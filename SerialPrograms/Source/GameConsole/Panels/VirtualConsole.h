/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_VirtualConsole_H
#define PokemonAutomation_ConsoleInfra_VirtualConsole_H

#include "GameConsole/ConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{



class VirtualConsole_Descriptor : public ConsolePanelDescriptor{
public:
    VirtualConsole_Descriptor()
        : ConsolePanelDescriptor(
            Color(),
            "GameConsole:VirtualConsole",
            "Game Console",
            "Virtual Console",
            "Programs/NintendoSwitch/VirtualConsole.html",
            "Play your console from your computer.",
            1
        )
    {}
};


class MultiControllerTester_Descriptor : public ConsolePanelDescriptor{
public:
    MultiControllerTester_Descriptor()
        : ConsolePanelDescriptor(
            Color(),
            "GameConsole:MultiControllerTester",
            "Game Console",
            "Multi-Controller Tester",
            "Programs/NintendoSwitch/VirtualConsole.html",
            "Test multiple controllers at once.",
            8
        )
    {}
};






}
}
#endif

