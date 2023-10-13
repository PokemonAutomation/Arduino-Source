/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
//#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
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
        "60 * TICKS_PER_SECOND"
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
        "60 * TICKS_PER_SECOND"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
    , RAID_SPAWN_DELAY(
        "<b>Raid Spawn Delay</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , SHINY_SOUND_THRESHOLD2(
        "<b>Shiny Sound Threshold:</b><br>Maximum error coefficient to trigger a shiny detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.96, 0, 1.0
    )
    , SHINY_SOUND_LOW_FREQUENCY(
        "<b>Shiny Sound Low Frequency (Hz):</b><br>High pass filter frequency for shiny sound.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 0, 48000
    )
    , LETS_GO_KILL_SOUND_THRESHOLD(
        "<b>Let's Go Kill Sound Threshold:</b><br>Maximum error coefficient to trigger a Let's Go Kill detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.93, 0, 1.0
    )
    , LETS_GO_KILL_SOUND_LOW_FREQUENCY(
        "<b>Let's Go Kill Sound Low Frequency (Hz):</b><br>High pass filter frequency for Let's Go Kill sound.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 0, 48000
    )
{
    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH);
    PA_ADD_OPTION(START_GAME_WAIT);
    PA_ADD_OPTION(ENTER_GAME_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);
    PA_ADD_OPTION(m_advanced_options);
    PA_ADD_OPTION(RAID_SPAWN_DELAY);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD2);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);
    PA_ADD_OPTION(LETS_GO_KILL_SOUND_THRESHOLD);
    PA_ADD_OPTION(LETS_GO_KILL_SOUND_LOW_FREQUENCY);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonSV:GlobalSettings",
        STRING_POKEMON + " SV", STRING_POKEMON + " Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonSV/PokemonSettings.md",
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






