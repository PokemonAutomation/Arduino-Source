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


const Resolution DEFAULT_RESOLUTION(1920, 1080);


ConsoleSettings& ConsoleSettings::instance(){
    static ConsoleSettings settings;
    return settings;
}
ConsoleSettings::ConsoleSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , SETTINGS_TO_HOME_DELAY0(
        "<b>Settings to Home Delay:</b><br>Delay from pressing home anywhere in the settings to return to the home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "960ms"
    )
    , START_GAME_REQUIRES_INTERNET(
        "<b>Start Game Requires Internet:</b><br>"
        "Set this to true if starting the game requires checking the internet. "
        "Otherwise, programs that require soft-resetting may not work properly.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , START_GAME_INTERNET_CHECK_DELAY0(
        "<b>Start Game Internet Check Delay:</b><br>"
        "If starting the game requires checking the internet, wait this long for it.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000ms"
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_FAST(
        "<b>Tolerate System Update Menu (fast):</b><br>"
        "Some programs can bypass the system update menu at little performance cost. Setting this to true enables this.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_SLOW(
        "<b>Tolerate System Update Menu (slow):</b><br"
        ">Some programs can bypass the system update menu, but will take a noticeable performance hit. "
        "Setting this to true enables this.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , KEYBOARD_SECTION("<font size=4><b>Keyboard to Controller Mappings:</b></font>")
{
    PA_ADD_OPTION(SETTINGS_TO_HOME_DELAY0);
    PA_ADD_OPTION(START_GAME_REQUIRES_INTERNET);
    PA_ADD_OPTION(START_GAME_INTERNET_CHECK_DELAY0);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    PA_ADD_STATIC(KEYBOARD_SECTION);
    PA_ADD_OPTION(KEYBOARD_MAPPINGS);
}


ConsoleSettings_Descriptor::ConsoleSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "NintendoSwitch:GlobalSettings",
        "Nintendo Switch", "Framework Settings",
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






