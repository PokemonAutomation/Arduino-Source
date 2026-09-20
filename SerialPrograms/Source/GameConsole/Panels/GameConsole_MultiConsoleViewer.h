/*  Multi-Console Viewer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GameConsole_MultiConsoleViewer_H
#define PokemonAutomation_GameConsole_MultiConsoleViewer_H

#include "GameConsole/MultiConsolePanel.h"

namespace PokemonAutomation{
namespace GameConsole{



class MultiConsoleViewer_Descriptor : public MultiConsolePanelDescriptor{
public:
    MultiConsoleViewer_Descriptor()
        : MultiConsolePanelDescriptor(
            "GameConsole:MultiConsoleViewer",
            "Game Console",
            "Multi-Console Viewer",
            "Programs/NintendoSwitch/SwitchViewer.html",
            "View multiple consoles simultaneously. Use this to figure out your camera/audio/controller mappings.",
            Color(),
            PanelDeprecation::NOT_DEPRECATED,
            1, 4, 2,
            false
        )
    {}
};
class MultiConsoleViewer : public MultiConsolePanelInstance{
public:
    using Descriptor = MultiConsoleViewer_Descriptor;
};





}
}
#endif
