/*  Nintendo Switch Settings Panel
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "NintendoSwitch_Settings.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{

using namespace std::chrono_literals;


const Resolution DEFAULT_RESOLUTION(1920, 1080);



TimingOptions::TimingOptions()
    : GroupOption(
        "Controller Timing Options",
        LockMode::UNLOCK_WHILE_RUNNING,
        EnableMode::ALWAYS_ENABLED,
        true
    )
    , WIRED_MICROCONTROLLER(
        "<b>Wired Microcontroller Timing Variation:</b><br>"
        "Assume that wired microcontrollers have a timing variation of no greater than this. "
        "This is used to adjust button delays to ensure they go through correctly.",
        LockMode::LOCK_WHILE_RUNNING,
        0ms, 200ms, "0 ms"
    )
    , WIRELESS_ESP32(
        "<b>Wireless ESP32 Timing Variation:</b><br>"
        "Assume that wireless ESP32 controllers have a timing variation of no greater than this. "
        "This is used to adjust button delays to ensure they go through correctly.",
        LockMode::LOCK_WHILE_RUNNING,
        0ms, 200ms, "10 ms"
    )
    , SYSBOTBASE(
        "<b>sys-botbase 2 Timing Variation:</b><br>"
        "Assume that sys-botbase 2 controllers have a timing variation of no greater than this. "
        "This is used to adjust button delays to ensure they go through correctly.<br>"
        "sys-botbase 3 does not use this option since it is as stable as a wired microcontroller "
        "and is therefore categorized accordingly.",
        LockMode::LOCK_WHILE_RUNNING,
        0ms, 200ms, "150 ms"
    )
{
    PA_ADD_OPTION(WIRED_MICROCONTROLLER);
    PA_ADD_OPTION(WIRELESS_ESP32);
    PA_ADD_OPTION(SYSBOTBASE);
}





ConsoleSettings& ConsoleSettings::instance(){
    static ConsoleSettings settings;
    return settings;
}
ConsoleSettings::ConsoleSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , TRUST_USER_CONSOLE_SELECTION(
        "<b>Trust User Console Selection:</b><br>"
        "Trust that the user's selection for the console type (Switch 1 vs. Switch 2) is correct.<br>"
        "Do not cross-check it. Do not stop the program if it disagrees with the user selection.<br>"
        "Don't enable this option unless you are encountering issues.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , SETTINGS_TO_HOME_DELAY0(
        "<b>Settings to Home Delay:</b><br>Delay from pressing home anywhere in the settings to return to the home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "960 ms"
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
        "3000 ms"
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_FAST(
        "<b>Tolerate System Update Menu (fast):</b><br>"
        "Some programs can bypass the system update menu at little performance cost. Setting this to true enables this.",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , TOLERATE_SYSTEM_UPDATE_MENU_SLOW(
        "<b>Tolerate System Update Menu (slow):</b><br>"
        "Some programs can bypass the system update menu, but will take a noticeable performance hit. "
        "Setting this to true enables this.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , ENABLE_ESP32_RECONNECT(
        "<b>Enable ESP32 Wireless Reconnect:</b><br>"
        "<font color=\"orange\">This is a buggy feature that when enabled will:</font><br>"
        " 1. Allow an ESP32 controller to reconnect to the previously paired Switch when switching to it from another controller.<br>"
        " 2. Clicking \"Reset Ctrl.\" will soft-reset the controller and attempt to reconnect to the same Switch it is already paired with.<br>"
        " 3. SHIFT-clicking \"Reset Ctrl.\" will clear the pairing state and begin pairing again. "
        "This will allow it to pair with a different Switch as well as clearing any MAC address bans on the current Switch.<br>"
        "<font color=\"red\">As of this writing, this feature works perfectly on the Switch 2, but is broken on the Switch 1.</font>",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , ENABLE_SBB3_PINGS(
        "<b>Enable sys-botbase 3 Pings:</b><br>"
        "Use sys-botbase's ping command for latency measurement instead of \"getVersion\".",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , ENABLE_SBB3_LOGGING(
        "<b>Enable sys-botbase 3 Logging:</b><br>"
        "Tell sys-botbase 3 to log its activity to its own log.",
        LockMode::UNLOCK_WHILE_RUNNING,
        false
    )
    , SWITCH1_DIGIT_ENTRY0(false)
    , SWITCH1_KEYBOARD_ENTRY0(false)
    , SWITCH2_DIGIT_ENTRY0(true)
    , SWITCH2_KEYBOARD_ENTRY0(true)
    , KEYBOARD_SECTION("<font size=4><b>Keyboard to Controller Mappings:</b></font>")
{
    PA_ADD_OPTION(CONTROLLER_SETTINGS);
    PA_ADD_OPTION(TRUST_USER_CONSOLE_SELECTION);
    PA_ADD_OPTION(SETTINGS_TO_HOME_DELAY0);
    PA_ADD_OPTION(START_GAME_REQUIRES_INTERNET);
    PA_ADD_OPTION(START_GAME_INTERNET_CHECK_DELAY0);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    PA_ADD_OPTION(ENABLE_ESP32_RECONNECT);
    PA_ADD_OPTION(ENABLE_SBB3_PINGS);
    PA_ADD_OPTION(ENABLE_SBB3_LOGGING);
    PA_ADD_OPTION(TIMING_OPTIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(SWITCH1_DIGIT_ENTRY0);
        PA_ADD_OPTION(SWITCH1_KEYBOARD_ENTRY0);
        PA_ADD_OPTION(SWITCH2_DIGIT_ENTRY0);
        PA_ADD_OPTION(SWITCH2_KEYBOARD_ENTRY0);
        PA_ADD_OPTION(KEYBOARD_CONTROLLER_TIMINGS);
    }
    PA_ADD_STATIC(KEYBOARD_SECTION);
    PA_ADD_OPTION(KEYBOARD_MAPPINGS);
}
void ConsoleSettings::load_json(const JsonValue& json){
    if (m_loaded){
        return;
    }
    m_loaded = true;
    BatchOption::load_json(json);
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






