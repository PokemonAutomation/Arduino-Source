/*  Pokemon Sword/Shield Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : m_egg_options("<font size=4><b>Egg Options:</b></font>")
    , AUTO_DEPOSIT(
        "<b>Auto-Deposit:</b><br>true = Send " + STRING_POKEMON + " to boxes is \"Automatic\".<br>false = Send " + STRING_POKEMON + " to boxes is \"Manual\".",
        true
    )
    , EGG_FETCH_EXTRA_LINE(
        "<b>Egg Fetch Extra Line:</b><br>true = The daycare lady has an extra line of text in Japanese. Set this to true if you are running any of the egg programs in a Japanese game.",
        false
    )
    , FETCH_EGG_MASH_DELAY(
        "<b>Fetch Egg Mash Delay:</b><br>Time needed to mash B to fetch an egg and return to overworld when auto-deposit is on.",
        "800"
    )
    , m_den_options("<font size=4><b>Den Options:</b></font>")
    , DODGE_UNCATCHABLE_PROMPT_FAST(
        "<b>Dodge Uncatchable Prompt Fast:</b><br>Which method to use to bypass the uncatchable " + STRING_POKEMON + " prompt?<br>true = Use a fast (but potentially unreliable) method.<br>false = Use a slower (by about 5 seconds) method.",
        false
    )
    , m_advanced_options("<font size=5><b>Advanced Options:</b></font> You shouldn't need to touch anything below here.")
    , m_general_options("<font size=4><b>General Timings:</b></font>")
    , AUTO_FR_DURATION(
        "<b>Auto-FR Duration:</b><br>Time to accept FRs before returning to den lobby.",
        "8 * TICKS_PER_SECOND"
    )
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , OVERWORLD_TO_MENU_DELAY(
        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
        "120"
    )
    , MENU_TO_OVERWORLD_DELAY(
        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
        "250"
    )
    , GAME_TO_HOME_DELAY_FAST(
        "<b>Game to Home Delay (fast):</b><br>Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        "100"
    )
    , GAME_TO_HOME_DELAY_SAFE(
        "<b>Game to Home Delay (safe):</b><br>Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        "125"
    )
    , HOME_TO_GAME_DELAY(
        "<b>Home to Game Delay:</b><br>Delay to enter game from home menu.",
        "3 * TICKS_PER_SECOND"
    )
    , OPEN_YCOMM_DELAY(
        "<b>Open Y-COMM Delay:</b><br>Time needed to open Y-COMM.",
        "200"
    )
    , ENTER_PROFILE_DELAY(
        "<b>Enter Profile Delay:</b><br>Delay to enter your Switch profile.",
        "2 * TICKS_PER_SECOND"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        "2 * TICKS_PER_SECOND"
    )
    , START_GAME_WAIT(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        "20 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_MASH(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        "3 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        "11 * TICKS_PER_SECOND"
    )
    , m_den_timings("<font size=4><b>Den Timings:</b></font>")
    , ENTER_OFFLINE_DEN_DELAY(
        "<b>Enter Offline Game Delay:</b><br>Time needed to enter a den on final button press.",
        "125"
    )
    , REENTER_DEN_DELAY(
        "<b>Re-enter Den Delay:</b><br>Time from exiting den after a day-skip to when you can collect watts and re-enter it.",
        "5 * TICKS_PER_SECOND"
    )
    , COLLECT_WATTS_OFFLINE_DELAY(
        "<b>Collect Watts Delay (offline):</b>",
        "80"
    )
    , COLLECT_WATTS_ONLINE_DELAY(
        "<b>Collect Watts Delay (online):</b>",
        "3 * TICKS_PER_SECOND"
    )
    , UNCATCHABLE_PROMPT_DELAY(
        "<b>Uncatchable Prompt Delay:</b><br>Time needed to bypass uncatchable pokemon prompt.",
        "110"
    )
    , OPEN_LOCAL_DEN_LOBBY_DELAY(
        "<b>Open Local Den Lobby Delay:</b><br>Time needed to open a den lobby on local connection.",
        "3 * TICKS_PER_SECOND"
    )
    , ENTER_SWITCH_POKEMON(
        "<b>Enter Switch " + STRING_POKEMON + ":</b><br>Time needed to enter Switch " + STRING_POKEMON + ".",
        "4 * TICKS_PER_SECOND"
    )
    , EXIT_SWITCH_POKEMON(
        "<b>Exit Switch " + STRING_POKEMON + ":</b><br>Time needed to exit Switch " + STRING_POKEMON + " back to den lobby.",
        "3 * TICKS_PER_SECOND"
    )
    , FULL_LOBBY_TIMER(
        "<b>Full Lobby Timer:</b><br>Always 3 minutes.",
        "180 * TICKS_PER_SECOND"
    )
    , m_box_timings("<font size=4><b>Box Timings:</b></font> (for egg programs)")
    , BOX_SCROLL_DELAY(
        "<b>Box Scroll Delay:</b><br>Delay to move the cursor.",
        "20"
    )
    , BOX_CHANGE_DELAY(
        "<b>Box Change Delay:</b><br>Delay to change boxes.",
        "80"
    )
    , BOX_PICKUP_DROP_DELAY(
        "<b>Box Pickup/Drop Delay:</b><br>Delay to pickup/drop " + STRING_POKEMON + ".",
        "90"
    )
    , MENU_TO_POKEMON_DELAY(
        "<b>Menu To " + STRING_POKEMON + " Delay:</b><br>Delay to enter " + STRING_POKEMON + " menu.",
        "300"
    )
    , POKEMON_TO_BOX_DELAY(
        "<b>" + STRING_POKEMON + " to Box Delay:</b><br>Delay to enter box system.",
        "300"
    )
    , BOX_TO_POKEMON_DELAY(
        "<b>Box to " + STRING_POKEMON + " Delay:</b><br>Delay to exit box system.",
        "250"
    )
    , POKEMON_TO_MENU_DELAY(
        "<b>" + STRING_POKEMON + " to Menu Delay:</b><br>Delay to return to menu.",
        "250"
    )
    , m_experimental("<font size=4><b>Experimental/Beta Features:</b></font>")
    , USE_NEW_SHINY_DETECTOR(
        "<b>Use New Shiny Detector:</b><br>Use the new version of the shiny detection algorithm.",
        true
    )
{
    PA_ADD_STATIC(m_egg_options);
    PA_ADD_OPTION(AUTO_DEPOSIT);
    PA_ADD_OPTION(EGG_FETCH_EXTRA_LINE);
    PA_ADD_OPTION(FETCH_EGG_MASH_DELAY);

    PA_ADD_STATIC(m_den_options);
    PA_ADD_OPTION(DODGE_UNCATCHABLE_PROMPT_FAST);

    PA_ADD_STATIC(m_advanced_options);

    PA_ADD_STATIC(m_general_options);
    PA_ADD_OPTION(AUTO_FR_DURATION);

    PA_ADD_STATIC(m_menu_navigation);
    PA_ADD_OPTION(OVERWORLD_TO_MENU_DELAY);
    PA_ADD_OPTION(MENU_TO_OVERWORLD_DELAY);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY_FAST);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY_SAFE);
    PA_ADD_OPTION(HOME_TO_GAME_DELAY);
    PA_ADD_OPTION(OPEN_YCOMM_DELAY);
    PA_ADD_OPTION(ENTER_PROFILE_DELAY);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH);
    PA_ADD_OPTION(START_GAME_WAIT);
    PA_ADD_OPTION(ENTER_GAME_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);

    PA_ADD_STATIC(m_den_timings);
    PA_ADD_OPTION(ENTER_OFFLINE_DEN_DELAY);
    PA_ADD_OPTION(REENTER_DEN_DELAY);
    PA_ADD_OPTION(COLLECT_WATTS_OFFLINE_DELAY);
    PA_ADD_OPTION(COLLECT_WATTS_ONLINE_DELAY);
    PA_ADD_OPTION(UNCATCHABLE_PROMPT_DELAY);
    PA_ADD_OPTION(OPEN_LOCAL_DEN_LOBBY_DELAY);
    PA_ADD_OPTION(ENTER_SWITCH_POKEMON);
    PA_ADD_OPTION(EXIT_SWITCH_POKEMON);
    PA_ADD_OPTION(FULL_LOBBY_TIMER);

    PA_ADD_STATIC(m_box_timings);
    PA_ADD_OPTION(BOX_SCROLL_DELAY);
    PA_ADD_OPTION(BOX_CHANGE_DELAY);
    PA_ADD_OPTION(BOX_PICKUP_DROP_DELAY);
    PA_ADD_OPTION(MENU_TO_POKEMON_DELAY);
    PA_ADD_OPTION(POKEMON_TO_BOX_DELAY);
    PA_ADD_OPTION(BOX_TO_POKEMON_DELAY);
    PA_ADD_OPTION(POKEMON_TO_MENU_DELAY);

    PA_ADD_STATIC(m_experimental);
    PA_ADD_OPTION(USE_NEW_SHINY_DETECTOR);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonSwSh:GlobalSettings",
        STRING_POKEMON + " SwSh", STRING_POKEMON + " Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/PokemonSettings.md",
        "Global " + STRING_POKEMON + " Settings"
    )
{}



GameSettingsPanel::GameSettingsPanel(const GameSettings_Descriptor& descriptor)
    : SettingsPanelInstance(descriptor)
    , settings(GameSettings::instance())
{
    PA_ADD_OPTION(settings);
}





}
}
}






