/*  Virtual Game Console
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ConsoleInfra_VirtualConsole_H
#define PokemonAutomation_ConsoleInfra_VirtualConsole_H

#include "ConsoleInfra/ConsolePanel.h"

namespace PokemonAutomation{
namespace ConsoleInfra{



class VirtualConsole_Descriptor : public ConsolePanelDescriptor{
public:
    VirtualConsole_Descriptor()
        : ConsolePanelDescriptor(
            Color(),
            "CC:VirtualConsole",
            "CC",
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
            "CC:MultiControllerTester",
            "CC",
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

