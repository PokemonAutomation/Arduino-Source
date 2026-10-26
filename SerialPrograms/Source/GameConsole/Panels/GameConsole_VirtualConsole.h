/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_VirtualConsole_H
#define PokemonAutomation_GameConsole_VirtualConsole_H

#include "GameConsole/ConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{



class VirtualConsole_Descriptor : public ConsolePanelDescriptor{
public:
    VirtualConsole_Descriptor()
        : ConsolePanelDescriptor(
            "GameConsole:VirtualConsole",
            "Game Console",
            "Virtual Console",
            "Programs/NintendoSwitch/VirtualConsole.html",
            "Play your console from your computer.",
            PanelQualityStanding::OFFICIAL,
            false,
            1
        )
    {}
};
class VirtualConsole : public ConsolePanelInstance{
public:
    using Descriptor = VirtualConsole_Descriptor;
};



class MultiControllerTester_Descriptor : public ConsolePanelDescriptor{
public:
    MultiControllerTester_Descriptor()
        : ConsolePanelDescriptor(
            "GameConsole:MultiControllerTester",
            "Game Console",
            "Multi-Controller Tester",
            "Programs/NintendoSwitch/VirtualConsole.html",
            "Test multiple controllers at once.",
            PanelQualityStanding::OFFICIAL,
            false,
            8
        )
    {}
};
class MultiControllerTester : public ConsolePanelInstance{
public:
    using Descriptor = MultiControllerTester_Descriptor;
};






}
}
#endif

