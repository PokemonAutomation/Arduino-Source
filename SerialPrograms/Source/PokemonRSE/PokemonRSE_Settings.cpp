/*  Pokemon RSE Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"

#include "PokemonRSE_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



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
    , m_soft_reset_timings("<font size=4><b>Soft Reset Timings:</b></font>")
    , START_BUTTON_MASH(
        "<b>1. Start Button Mash:</b><br>Mash Start for this long after a soft reset to get to the main menu.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "4 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>2. Enter Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        TICKS_PER_SECOND,
        "3 * TICKS_PER_SECOND"
    )
    , m_shiny_audio_settings("<font size=4><b>Shiny Audio Settings:</b></font>")
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
{
    PA_ADD_STATIC(m_soft_reset_timings);
    PA_ADD_OPTION(START_BUTTON_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);
    PA_ADD_STATIC(m_shiny_audio_settings);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonRSE:GlobalSettings",
        "Pokemon RSE", "Pokemon Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonRSE/RSESettings.md",
        "Global Pokemon RSE Settings"
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






