/*  Pokemon BDSP Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonBDSP_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{
    using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , OVERWORLD_TO_MENU_DELAY(
        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "250"
    )
    , MENU_TO_OVERWORLD_DELAY(
        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "250"
    )
    , GAME_TO_HOME_DELAY(
        "<b>Game to Home Delay:</b><br>Delay from pressing home to entering the the Switch home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "125"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "2 * TICKS_PER_SECOND"
    )
    , START_GAME_WAIT(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "40 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_MASH(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "5 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "40 * TICKS_PER_SECOND"
    )
    , m_box_timings("<font size=4><b>Box Timings:</b></font> (for egg programs)")
    , BOX_SCROLL_DELAY_0(
        "<b>Box Scroll Delay:</b><br>Delay to move the cursor.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "30"
    )
    , BOX_CHANGE_DELAY_0(
        "<b>Box Change Delay:</b><br>Delay to change boxes.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "200"
    )
    , BOX_PICKUP_DROP_DELAY(
        "<b>Box Pickup/Drop Delay:</b><br>Delay to pickup/drop " + STRING_POKEMON + ".",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "50"
    )
    , MENU_TO_POKEMON_DELAY(
        "<b>Menu To " + STRING_POKEMON + " Delay:</b><br>Delay to enter " + STRING_POKEMON + " menu.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "300"
    )
    , POKEMON_TO_BOX_DELAY0(
        "<b>" + STRING_POKEMON + " to Box Delay:</b><br>Delay to enter box system.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "320"
    )
    , BOX_TO_POKEMON_DELAY(
        "<b>Box to " + STRING_POKEMON + " Delay:</b><br>Delay to exit box system.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "250"
    )
    , POKEMON_TO_MENU_DELAY(
        "<b>" + STRING_POKEMON + " to Menu Delay:</b><br>Delay to return to menu.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "250"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , SHINY_ALPHA_OVERALL_THRESHOLD(
        "<b>Shiny Threshold (overall):</b><br>Threshold to detect a shiny encounter.",
        LockMode::LOCK_WHILE_RUNNING,
        4.0, 0
    )
    , SHINY_ALPHA_SIDE_THRESHOLD(
        "<b>Shiny Threshold (left/right):</b><br>Threshold to detect a left/right shiny.",
        LockMode::LOCK_WHILE_RUNNING,
        3.0, 0
    )
    , BALL_SPARKLE_ALPHA(
        "<b>Ball Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1.0, 0
    )
    , STAR_SPARKLE_ALPHA(
        "<b>Star Sparkle Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        1.0, 0
    )
    , SHINY_DIALOG_ALPHA(
        "<b>Shiny Dialog Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        3.5, 0
    )
    , SHINY_SOUND_THRESHOLD(
        "<b>Shiny Sound Threshold:</b><br>Maximum error coefficient to trigger a shiny detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.87, 0, 1.0
    )
    , SHINY_SOUND_LOW_FREQUENCY(
        "<b>Shiny Sound Low Frequency (Hz):</b><br>High pass filter frequency for shiny sound.",
        LockMode::LOCK_WHILE_RUNNING,
        5000, 0, 48000
    )
    , SHINY_SOUND_ALPHA(
        "<b>Shiny Sound Alpha:</b>",
        LockMode::LOCK_WHILE_RUNNING,
        5.0, 0
    )
//    , m_experimental("<font size=4><b>Experimental/Beta Features:</b></font>")
{
    PA_ADD_STATIC(m_menu_navigation);
    PA_ADD_OPTION(OVERWORLD_TO_MENU_DELAY);
    PA_ADD_OPTION(MENU_TO_OVERWORLD_DELAY);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH);
    PA_ADD_OPTION(START_GAME_WAIT);
    PA_ADD_OPTION(ENTER_GAME_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);

    PA_ADD_STATIC(m_box_timings);
    PA_ADD_OPTION(BOX_SCROLL_DELAY_0);
    PA_ADD_OPTION(BOX_CHANGE_DELAY_0);
    PA_ADD_OPTION(BOX_PICKUP_DROP_DELAY);
    PA_ADD_OPTION(MENU_TO_POKEMON_DELAY);
    PA_ADD_OPTION(POKEMON_TO_BOX_DELAY0);
    PA_ADD_OPTION(BOX_TO_POKEMON_DELAY);
    PA_ADD_OPTION(POKEMON_TO_MENU_DELAY);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SHINY_ALPHA_OVERALL_THRESHOLD);
    PA_ADD_OPTION(SHINY_ALPHA_SIDE_THRESHOLD);
    PA_ADD_OPTION(BALL_SPARKLE_ALPHA);
    PA_ADD_OPTION(STAR_SPARKLE_ALPHA);
    PA_ADD_OPTION(SHINY_DIALOG_ALPHA);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);
    PA_ADD_OPTION(SHINY_SOUND_ALPHA);

//    PA_ADD_STATIC(m_experimental);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonBDSP:GlobalSettings",
        STRING_POKEMON + " BDSP", STRING_POKEMON + " Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonBDSP/PokemonSettings.md",
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






