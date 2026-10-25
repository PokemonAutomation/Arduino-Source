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
    : OptionsPanelDescriptor(
        "NintendoSwitch:GlobalSettings",
        "Nintendo Switch", "Nintendo Switch Settings",
        "Programs/NintendoSwitch/FrameworkSettings.html",
        "Switch Framework Settings",
        PanelQualityStanding::OFFICIAL
    )
{}

ConsoleSettingsPanel::ConsoleSettingsPanel()
    : settings(ConsoleSettings::instance())
{
    PA_ADD_OPTION(settings);
}




}
}






