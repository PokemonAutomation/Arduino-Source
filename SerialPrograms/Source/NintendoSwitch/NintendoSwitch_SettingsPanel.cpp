/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "NintendoSwitch_SettingsPanel.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


ConsoleSettings_Descriptor::ConsoleSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "NintendoSwitch:GlobalSettings",
        "Nintendo Switch", "Framework Settings",
        "Programs/NintendoSwitch/FrameworkSettings.html",
        "Switch Framework Settings"
    )
{}

ConsoleSettingsPanel::ConsoleSettingsPanel(const ConsoleSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , settings(ConsoleSettings::instance())
{
    PA_ADD_OPTION(settings);
}




}
}






