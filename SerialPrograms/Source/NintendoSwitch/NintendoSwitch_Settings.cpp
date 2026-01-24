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
    , WIRED(
        "<b>Wired Controller Timing Variation:</b><br>"
        "Assume that wired microcontrollers have a timing variation of no greater than this. "
        "This is used to adjust button delays to ensure they go through correctly.",
        LockMode::LOCK_WHILE_RUNNING,
        0ms, 200ms, "0 ms"
    )
    , WIRELESS(
        "<b>Wireless Timing Variation:</b><br>"
        "Assume that wireless controllers have a timing variation of no greater than this. "
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
    PA_ADD_OPTION(WIRED);
    PA_ADD_OPTION(WIRELESS);
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
    , START_GAME_MASH(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
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
    , CODEBOARD_ENTRY_SWITCH1_WIRED("Fast Code Entry Timings (Switch 1 Wired Controller)",          false, false)
    , CODEBOARD_ENTRY_SWITCH1_WIRELESS("Fast Code Entry Timings (Switch 1 Wireless Controller)",    false, true)
    , CODEBOARD_ENTRY_SWITCH2_WIRED("Fast Code Entry Timings (Switch 2 Wired Controller)",          true, false)
    , CODEBOARD_ENTRY_SWITCH2_WIRELESS("Fast Code Entry Timings (Switch 2 Wireless Controller)",    true, true)
    , KEYBOARD_SECTION("<font size=4><b>Keyboard to Controller Mappings:</b></font>")
{
    PA_ADD_OPTION(CONTROLLER_SETTINGS);
    PA_ADD_OPTION(TRUST_USER_CONSOLE_SELECTION);
    PA_ADD_OPTION(START_GAME_MASH);
    PA_ADD_OPTION(SETTINGS_TO_HOME_DELAY0);
    PA_ADD_OPTION(START_GAME_REQUIRES_INTERNET);
    PA_ADD_OPTION(START_GAME_INTERNET_CHECK_DELAY0);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_FAST);
    PA_ADD_OPTION(TOLERATE_SYSTEM_UPDATE_MENU_SLOW);
    PA_ADD_OPTION(ENABLE_SBB3_PINGS);
    PA_ADD_OPTION(ENABLE_SBB3_LOGGING);
    PA_ADD_OPTION(TIMING_OPTIONS);
    if (PreloadSettings::instance().DEVELOPER_MODE){
        PA_ADD_OPTION(CODEBOARD_ENTRY_SWITCH1_WIRED);
        PA_ADD_OPTION(CODEBOARD_ENTRY_SWITCH1_WIRELESS);
        PA_ADD_OPTION(CODEBOARD_ENTRY_SWITCH2_WIRED);
        PA_ADD_OPTION(CODEBOARD_ENTRY_SWITCH2_WIRELESS);
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






