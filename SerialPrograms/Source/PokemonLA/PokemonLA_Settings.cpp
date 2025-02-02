/*  Pokemon Legends Arceus Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{
    using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_general("<font size=4><b>General Settings:</b></font>")
    , POST_WARP_DELAY(
        "<b>Post-Warp Delay:</b><br>After warping, wait this many seconds before continuing.",
        LockMode::LOCK_WHILE_RUNNING,
        1.0, 0, 100
    )
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
//    , OVERWORLD_TO_MENU_DELAY(
//        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
//        "250"
//    )
//    , MENU_TO_OVERWORLD_DELAY(
//        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
//        "250"
//    )
    , GAME_TO_HOME_DELAY(
        "<b>Game to Home Delay:</b><br>Delay from pressing home to entering the the Switch home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "125"
    )
    , LOAD_REGION_TIMEOUT(
        "<b>Load Region Timeout:</b><br>Wait at most this long to enter a region before giving up.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "30 * TICKS_PER_SECOND"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH0(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000ms"
    )
    , START_GAME_WAIT1(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "40000ms"
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
        "15 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
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
    , ALPHA_ROAR_THRESHOLD(
        "<b>Alpha Roar Threshold:</b><br>Maximum error coefficient to trigger an alpha roar detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.65, 0, 1.0
    )
    , ALPHA_MUSIC_THRESHOLD(
        "<b>Alpha Music Threshold:</b><br>Maximum error coefficient to trigger an alpha music detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.81, 0, 1.0
    )
    , ITEM_DROP_SOUND_THRESHOLD(
        "<b>Item Drop Sound Threshold:</b><br>Maximum error coefficient to trigger an item drop sound detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.9, 0, 1.0
    )
    , ITEM_DROP_SOUND_LOW_FREQUENCY(
        "<b>Item Drop Sound Low Frequency (Hz):</b><br>High pass filter frequency for item drop sound.",
        LockMode::LOCK_WHILE_RUNNING,
        5000, 0, 48000
    )
{
    PA_ADD_STATIC(m_general);
    PA_ADD_OPTION(POST_WARP_DELAY);

    PA_ADD_STATIC(m_menu_navigation);
//    PA_ADD_OPTION(OVERWORLD_TO_MENU_DELAY);
//    PA_ADD_OPTION(MENU_TO_OVERWORLD_DELAY);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY);
    PA_ADD_OPTION(LOAD_REGION_TIMEOUT);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH0);
    PA_ADD_OPTION(START_GAME_WAIT1);
    PA_ADD_OPTION(ENTER_GAME_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);

    PA_ADD_STATIC(m_advanced_options);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);
    PA_ADD_OPTION(ALPHA_ROAR_THRESHOLD);
    PA_ADD_OPTION(ALPHA_MUSIC_THRESHOLD);
    PA_ADD_OPTION(ITEM_DROP_SOUND_THRESHOLD);
    PA_ADD_OPTION(ITEM_DROP_SOUND_LOW_FREQUENCY);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonLA:GlobalSettings",
        STRING_POKEMON + " LA", STRING_POKEMON + " Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLA/PokemonSettings.md",
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






