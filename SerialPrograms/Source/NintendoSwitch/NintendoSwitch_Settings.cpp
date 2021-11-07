/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "NintendoSwitch_Settings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


ConsoleSettings& ConsoleSettings::instance(){
    static ConsoleSettings settings;
    return settings;
}
ConsoleSettings::ConsoleSettings()
    : CONNECT_CONTROLLER_DELAY(
        "<b>Connection Controller Delay:</b><br>Wait this long before starting the program. The LEDs normally flash during this time.",
        "5 * TICKS_PER_SECOND"
    )
    , SETTINGS_TO_HOME_DELAY(
        "<b>Settings to Home Delay:</b><br>Delay from pressing home anywhere in the settings to return to the home menu.",
        "120"
    )
    , START_GAME_REQUIRES_INTERNET(
        "<b>Start Game Requires Internet:</b><br>Set this to true if starting the game requires checking the internet. Otherwise, programs that require soft-resetting may not work properly.",
        true
    )
    , START_GAME_INTERNET_CHECK_DELAY(
        "<b>Start Game Internet Check Delay:</b><br>If starting the game requires checking the internet, wait this long for it.",
        "3 * TICKS_PER_SECOND"
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_FAST(
        "<b>Tolerate System Update Menu (fast):</b><br>Some programs can bypass the system update menu at little performance cost. Setting this to true enables this.",
        true
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_SLOW(
        "<b>Tolerate System Update Menu (slow):</b><br>Some programs can bypass the system update menu, but will take a noticeable performance hit. Setting this to true enables this.",
        false
    )
{
    PA_ADD_OPTION(CONNECT_CONTROLLER_DELAY);
    PA_ADD_OPTION(SETTINGS_TO_HOME_DELAY);
    PA_ADD_OPTION(START_GAME_REQUIRES_INTERNET);
    PA_ADD_OPTION(START_GAME_INTERNET_CHECK_DELAY);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
}


ConsoleSettings_Descriptor::ConsoleSettings_Descriptor()
    : PanelDescriptor(
        QColor(),
        "NintendoSwitch:GlobalSettings",
        "Framework Settings",
        "ComputerControl/blob/master/Wiki/Programs/NintendoSwitch/FrameworkSettings.md",
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






