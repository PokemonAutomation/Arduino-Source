/*  Pokemon Sword/Shield Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSwSh_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{
    using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockWhileRunning::LOCK_WHILE_RUNNING)
    , m_egg_options("<font size=4><b>Egg Options:</b></font>")
    , AUTO_DEPOSIT(
        "<b>Auto-Deposit:</b><br>true = Send " + STRING_POKEMON + " to boxes is \"Automatic\".<br>false = Send " + STRING_POKEMON + " to boxes is \"Manual\".",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        true
    )
    , EGG_FETCH_EXTRA_LINE(
        "<b>Egg Fetch Extra Line:</b><br>true = The daycare lady has an extra line of text in Japanese. Set this to true if you are running any of the egg programs in a Japanese game.",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        false
    )
    , FETCH_EGG_MASH_DELAY(
        "<b>Fetch Egg Mash Delay:</b><br>Time needed to mash B to fetch an egg and return to overworld when auto-deposit is on.",
        TICKS_PER_SECOND,
        "800"
    )
    , m_den_options("<font size=4><b>Den Options:</b></font>")
    , DODGE_UNCATCHABLE_PROMPT_FAST(
        "<b>Dodge Uncatchable Prompt Fast:</b><br>Which method to use to bypass the uncatchable " + STRING_POKEMON + " prompt?<br>true = Use a fast (but potentially unreliable) method.<br>false = Use a slower (by about 5 seconds) method.",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        false
    )
    , m_advanced_options("<font size=5><b>Advanced Options:</b></font> You shouldn't need to touch anything below here.")
    , m_general_options("<font size=4><b>General Timings:</b></font>")
    , AUTO_FR_DURATION(
        "<b>Auto-FR Duration:</b><br>Time to accept FRs before returning to den lobby.",
        TICKS_PER_SECOND,
        "8 * TICKS_PER_SECOND"
    )
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , OVERWORLD_TO_MENU_DELAY(
        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
        TICKS_PER_SECOND,
        "120"
    )
    , MENU_TO_OVERWORLD_DELAY(
        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
        TICKS_PER_SECOND,
        "250"
    )
    , GAME_TO_HOME_DELAY_FAST(
        "<b>Game to Home Delay (fast):</b><br>"
        "Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        TICKS_PER_SECOND,
        "100"
    )
    , GAME_TO_HOME_DELAY_SAFE(
        "<b>Game to Home Delay (safe):</b><br>"
        "Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        TICKS_PER_SECOND,
        "125"
    )
    , HOME_TO_GAME_DELAY(
        "<b>Home to Game Delay:</b><br>Delay to enter game from home menu.",
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , OPEN_YCOMM_DELAY(
        "<b>Open Y-COMM Delay:</b><br>Time needed to open Y-COMM.",
        TICKS_PER_SECOND,
        "200"
    )
    , ENTER_PROFILE_DELAY(
        "<b>Enter Profile Delay:</b><br>Delay to enter your Switch profile.",
        TICKS_PER_SECOND,
        "2 * TICKS_PER_SECOND"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        TICKS_PER_SECOND,
        "2 * TICKS_PER_SECOND"
    )
    , START_GAME_WAIT(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        TICKS_PER_SECOND,
        "20 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_MASH(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        TICKS_PER_SECOND,
        "11 * TICKS_PER_SECOND"
    )
    , m_den_timings("<font size=4><b>Den Timings:</b></font>")
    , ENTER_OFFLINE_DEN_DELAY(
        "<b>Enter Offline Game Delay:</b><br>Time needed to enter a den on final button press.",
        TICKS_PER_SECOND,
        "125"
    )
    , REENTER_DEN_DELAY(
        "<b>Re-enter Den Delay:</b><br>Time from exiting den after a day-skip to when you can collect watts and re-enter it.",
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
    , COLLECT_WATTS_OFFLINE_DELAY(
        "<b>Collect Watts Delay (offline):</b>",
        TICKS_PER_SECOND,
        "80"
    )
    , COLLECT_WATTS_ONLINE_DELAY(
        "<b>Collect Watts Delay (online):</b>",
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , UNCATCHABLE_PROMPT_DELAY(
        "<b>Uncatchable Prompt Delay:</b><br>Time needed to bypass uncatchable pokemon prompt.",
        TICKS_PER_SECOND,
        "110"
    )
    , OPEN_LOCAL_DEN_LOBBY_DELAY(
        "<b>Open Local Den Lobby Delay:</b><br>Time needed to open a den lobby on local connection.",
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , ENTER_SWITCH_POKEMON(
        "<b>Enter Switch " + STRING_POKEMON + ":</b><br>Time needed to enter Switch " + STRING_POKEMON + ".",
        TICKS_PER_SECOND,
        "4 * TICKS_PER_SECOND"
    )
    , EXIT_SWITCH_POKEMON(
        "<b>Exit Switch " + STRING_POKEMON + ":</b><br>Time needed to exit Switch " + STRING_POKEMON + " back to den lobby.",
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , FULL_LOBBY_TIMER(
        "<b>Full Lobby Timer:</b><br>Always 3 minutes.",
        TICKS_PER_SECOND,
        "180 * TICKS_PER_SECOND"
    )
    , m_box_timings("<font size=4><b>Box Timings:</b></font> (for egg programs)")
    , BOX_SCROLL_DELAY(
        "<b>Box Scroll Delay:</b><br>Delay to move the cursor.",
        TICKS_PER_SECOND,
        "20"
    )
    , BOX_CHANGE_DELAY(
        "<b>Box Change Delay:</b><br>Delay to change boxes.",
        TICKS_PER_SECOND,
        "80"
    )
    , BOX_PICKUP_DROP_DELAY(
        "<b>Box Pickup/Drop Delay:</b><br>Delay to pickup/drop " + STRING_POKEMON + ".",
        TICKS_PER_SECOND,
        "90"
    )
    , MENU_TO_POKEMON_DELAY(
        "<b>Menu To " + STRING_POKEMON + " Delay:</b><br>Delay to enter " + STRING_POKEMON + " menu.",
        TICKS_PER_SECOND,
        "300"
    )
    , POKEMON_TO_BOX_DELAY(
        "<b>" + STRING_POKEMON + " to Box Delay:</b><br>Delay to enter box system.",
        TICKS_PER_SECOND,
        "300"
    )
    , BOX_TO_POKEMON_DELAY(
        "<b>Box to " + STRING_POKEMON + " Delay:</b><br>Delay to exit box system.",
        TICKS_PER_SECOND,
        "250"
    )
    , POKEMON_TO_MENU_DELAY(
        "<b>" + STRING_POKEMON + " to Menu Delay:</b><br>Delay to return to menu.",
        TICKS_PER_SECOND,
        "250"
    )
    , m_shiny_detection("<font size=4><b>Shiny Detection:</b></font>")
    , SHINY_ALPHA_THRESHOLD(
        "<b>Shiny Threshold:</b><br>Threshold to detect a shiny encounter.",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        2.0, 0
    )
    , BALL_SPARKLE_ALPHA(
        "<b>Ball Sparkle Alpha:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        0.4, 0
    )
    , STAR_SPARKLE_ALPHA(
        "<b>Star Sparkle Alpha:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        0.5, 0
    )
    , SQUARE_SPARKLE_ALPHA(
        "<b>Ball Sparkle Alpha:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        0.3, 0
    )
    , LINE_SPARKLE_ALPHA(
        "<b>Star Sparkle Alpha:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        0.3, 0
    )
    , SHINY_DIALOG_ALPHA(
        "<b>Shiny Dialog Alpha:</b>",
        LockWhileRunning::LOCK_WHILE_RUNNING,
        1.2, 0
    )
//    , m_experimental("<font size=4><b>Experimental/Beta Features:</b></font>")
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

    PA_ADD_STATIC(m_shiny_detection);
    PA_ADD_OPTION(SHINY_ALPHA_THRESHOLD);
    PA_ADD_OPTION(BALL_SPARKLE_ALPHA);
    PA_ADD_OPTION(STAR_SPARKLE_ALPHA);
    PA_ADD_OPTION(SQUARE_SPARKLE_ALPHA);
    PA_ADD_OPTION(LINE_SPARKLE_ALPHA);
    PA_ADD_OPTION(SHINY_DIALOG_ALPHA);

//    PA_ADD_STATIC(m_experimental);
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






