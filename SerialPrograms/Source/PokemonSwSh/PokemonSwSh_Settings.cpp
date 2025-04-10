/*  Pokemon Sword/Shield Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_egg_options("<font size=4><b>Egg Options:</b></font>")
    , AUTO_DEPOSIT(
        "<b>Auto-Deposit:</b><br>true = Send " + STRING_POKEMON + " to boxes is \"Automatic\".<br>false = Send " + STRING_POKEMON + " to boxes is \"Manual\".",
        LockMode::LOCK_WHILE_RUNNING,
        true
    )
    , EGG_FETCH_EXTRA_LINE(
        "<b>Egg Fetch Extra Line:</b><br>true = The daycare lady has an extra line of text in Japanese. Set this to true if you are running any of the egg programs in a Japanese game.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , FETCH_EGG_MASH_DELAY0(
        "<b>Fetch Egg Mash Delay:</b><br>Time needed to mash B to fetch an egg and return to overworld when auto-deposit is on.",
        LockMode::LOCK_WHILE_RUNNING,
        "6400 ms"
    )
    , m_den_options("<font size=4><b>Den Options:</b></font>")
    , DODGE_UNCATCHABLE_PROMPT_FAST(
        "<b>Dodge Uncatchable Prompt Fast:</b><br>Which method to use to bypass the uncatchable " + STRING_POKEMON + " prompt?<br>true = Use a fast (but potentially unreliable) method.<br>false = Use a slower (by about 5 seconds) method.",
        LockMode::LOCK_WHILE_RUNNING,
        false
    )
    , m_advanced_options("<font size=5><b>Advanced Options:</b></font> You shouldn't need to touch anything below here.")
    , m_general_options("<font size=4><b>General Timings:</b></font>")
    , AUTO_FR_DURATION0(
        "<b>Auto-FR Duration:</b><br>Time to accept FRs before returning to den lobby.",
        LockMode::LOCK_WHILE_RUNNING,
        "8000 ms"
    )
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , OVERWORLD_TO_MENU_DELAY0(
        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "960 ms"
    )
    , MENU_TO_OVERWORLD_DELAY0(
        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , GAME_TO_HOME_DELAY_FAST0(
        "<b>Game to Home Delay (fast):</b><br>"
        "Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        LockMode::LOCK_WHILE_RUNNING,
        "800 ms"
    )
    , GAME_TO_HOME_DELAY_SAFE0(
        "<b>Game to Home Delay (safe):</b><br>"
        "Delay from pressing home to entering the the Switch home menu. This affects the speed of date-spamming programs.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , HOME_TO_GAME_DELAY0(
        "<b>Home to Game Delay:</b><br>Delay to enter game from home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , OPEN_YCOMM_DELAY0(
        "<b>Open Y-COMM Delay:</b><br>Time needed to open Y-COMM.",
        LockMode::LOCK_WHILE_RUNNING,
        "1600 ms"
    )
    , ENTER_PROFILE_DELAY0(
        "<b>Enter Profile Delay:</b><br>Delay to enter your Switch profile.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH0(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , START_GAME_WAIT0(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "20000 ms"
    )
    , ENTER_GAME_MASH0(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , ENTER_GAME_WAIT0(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "11000 ms"
    )
    , m_den_timings("<font size=4><b>Den Timings:</b></font>")
    , ENTER_OFFLINE_DEN_DELAY0(
        "<b>Enter Offline Game Delay:</b><br>Time needed to enter a den on final button press.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , REENTER_DEN_DELAY0(
        "<b>Re-enter Den Delay:</b><br>Time from exiting den after a day-skip to when you can collect watts and re-enter it.",
        LockMode::LOCK_WHILE_RUNNING,
        "5000 ms"
    )
    , COLLECT_WATTS_OFFLINE_DELAY0(
        "<b>Collect Watts Delay (offline):</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "640 ms"
    )
    , COLLECT_WATTS_ONLINE_DELAY0(
        "<b>Collect Watts Delay (online):</b>",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , UNCATCHABLE_PROMPT_DELAY0(
        "<b>Uncatchable Prompt Delay:</b><br>Time needed to bypass uncatchable pokemon prompt.",
        LockMode::LOCK_WHILE_RUNNING,
        "880 ms"
    )
    , OPEN_LOCAL_DEN_LOBBY_DELAY0(
        "<b>Open Local Den Lobby Delay:</b><br>Time needed to open a den lobby on local connection.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , ENTER_SWITCH_POKEMON0(
        "<b>Enter Switch " + STRING_POKEMON + ":</b><br>Time needed to enter Switch " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        "4000 ms"
    )
    , EXIT_SWITCH_POKEMON0(
        "<b>Exit Switch " + STRING_POKEMON + ":</b><br>Time needed to exit Switch " + STRING_POKEMON + " back to den lobby.",
        LockMode::LOCK_WHILE_RUNNING,
        "3000 ms"
    )
    , FULL_LOBBY_TIMER0(
        "<b>Full Lobby Timer:</b><br>Always 3 minutes.",
        LockMode::LOCK_WHILE_RUNNING,
        "180 s"
    )
    , m_box_timings("<font size=4><b>Box Timings:</b></font> (for egg programs)")
//    , BOX_SCROLL_DELAY1(
//        "<b>Box Scroll Delay:</b><br>Delay to move the cursor.",
//        LockMode::LOCK_WHILE_RUNNING,
//        "200 ms"
//    )
    , BOX_CHANGE_DELAY0(
        "<b>Box Change Delay:</b><br>Delay to change boxes.",
        LockMode::LOCK_WHILE_RUNNING,
        "640 ms"
    )
    , BOX_PICKUP_DROP_DELAY0(
        "<b>Box Pickup/Drop Delay:</b><br>Delay to pickup/drop " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        "720 ms"
    )
    , MENU_TO_POKEMON_DELAY0(
        "<b>Menu To " + STRING_POKEMON + " Delay:</b><br>Delay to enter " + STRING_POKEMON + " menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "2400 ms"
    )
    , POKEMON_TO_BOX_DELAY0(
        "<b>" + STRING_POKEMON + " to Box Delay:</b><br>Delay to enter box system.",
        LockMode::LOCK_WHILE_RUNNING,
        "2400 ms"
    )
    , BOX_TO_POKEMON_DELAY0(
        "<b>Box to " + STRING_POKEMON + " Delay:</b><br>Delay to exit box system.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , POKEMON_TO_MENU_DELAY0(
        "<b>" + STRING_POKEMON + " to Menu Delay:</b><br>Delay to return to menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , m_shiny_detection("<font size=4><b>Shiny Detection:</b></font>")
    , SHINY_ALPHA_THRESHOLD(
        "<b>Shiny Threshold:</b><br>Threshold to detect a shiny encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        2.0, 0
    )
    , BALL_SPARKLE_ALPHA(
        "<b>Ball Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0.4, 0
    )
    , STAR_SPARKLE_ALPHA(
        "<b>Star Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0.5, 0
    )
    , SQUARE_SPARKLE_ALPHA(
        "<b>Ball Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0.3, 0
    )
    , LINE_SPARKLE_ALPHA(
        "<b>Line Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        0.3, 0
    )
    , SHINY_DIALOG_ALPHA(
        "<b>Shiny Dialog Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1.2, 0
    )
//    , m_experimental("<font size=4><b>Experimental/Beta Features:</b></font>")
{
    PA_ADD_STATIC(m_egg_options);
    PA_ADD_OPTION(AUTO_DEPOSIT);
    PA_ADD_OPTION(EGG_FETCH_EXTRA_LINE);
    PA_ADD_OPTION(FETCH_EGG_MASH_DELAY0);

    PA_ADD_STATIC(m_den_options);
    PA_ADD_OPTION(DODGE_UNCATCHABLE_PROMPT_FAST);

    PA_ADD_STATIC(m_advanced_options);

    PA_ADD_STATIC(m_general_options);
    PA_ADD_OPTION(AUTO_FR_DURATION0);

    PA_ADD_STATIC(m_menu_navigation);
    PA_ADD_OPTION(OVERWORLD_TO_MENU_DELAY0);
    PA_ADD_OPTION(MENU_TO_OVERWORLD_DELAY0);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY_FAST0);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY_SAFE0);
    PA_ADD_OPTION(HOME_TO_GAME_DELAY0);
    PA_ADD_OPTION(OPEN_YCOMM_DELAY0);
    PA_ADD_OPTION(ENTER_PROFILE_DELAY0);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH0);
    PA_ADD_OPTION(START_GAME_WAIT0);
    PA_ADD_OPTION(ENTER_GAME_MASH0);
    PA_ADD_OPTION(ENTER_GAME_WAIT0);

    PA_ADD_STATIC(m_den_timings);
    PA_ADD_OPTION(ENTER_OFFLINE_DEN_DELAY0);
    PA_ADD_OPTION(REENTER_DEN_DELAY0);
    PA_ADD_OPTION(COLLECT_WATTS_OFFLINE_DELAY0);
    PA_ADD_OPTION(COLLECT_WATTS_ONLINE_DELAY0);
    PA_ADD_OPTION(UNCATCHABLE_PROMPT_DELAY0);
    PA_ADD_OPTION(OPEN_LOCAL_DEN_LOBBY_DELAY0);
    PA_ADD_OPTION(ENTER_SWITCH_POKEMON0);
    PA_ADD_OPTION(EXIT_SWITCH_POKEMON0);
    PA_ADD_OPTION(FULL_LOBBY_TIMER0);

    PA_ADD_STATIC(m_box_timings);
//    PA_ADD_OPTION(BOX_SCROLL_DELAY1);
    PA_ADD_OPTION(BOX_CHANGE_DELAY0);
    PA_ADD_OPTION(BOX_PICKUP_DROP_DELAY0);
    PA_ADD_OPTION(MENU_TO_POKEMON_DELAY0);
    PA_ADD_OPTION(POKEMON_TO_BOX_DELAY0);
    PA_ADD_OPTION(BOX_TO_POKEMON_DELAY0);
    PA_ADD_OPTION(POKEMON_TO_MENU_DELAY0);

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
        STRING_POKEMON + " SwSh", "Game Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSwSh/PokemonSettings.md",
        "Global " + STRING_POKEMON + " Sword and Shield Settings"
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






