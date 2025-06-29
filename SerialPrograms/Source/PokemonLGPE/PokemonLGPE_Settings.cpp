/*  Pokemon Let's Go Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLGPE_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{
    using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_general("<font size=4><b>General Settings:</b></font>")
    , m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , GAME_TO_HOME_DELAY0(
        "<b>Game to Home Delay:</b><br>Delay from pressing home to entering the the Switch home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH0(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "2000 ms"
    )
    , START_GAME_WAIT1(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "20 s"
    )
    , ENTER_GAME_MASH0(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        LockMode::LOCK_WHILE_RUNNING,
        "5 s"
    )
    , ENTER_GAME_WAIT0(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the opening animations to finish.",
        LockMode::LOCK_WHILE_RUNNING,
        "15 s"
    )
    , m_shiny_audio_settings("<font size=4><b>Shiny Audio Settings:</b></font>")
    , SHINY_SOUND_THRESHOLD(
        "<b>Shiny Sound Threshold:</b><br>Maximum error coefficient to trigger a shiny detection.",
        LockMode::LOCK_WHILE_RUNNING,
        0.95, 0, 1.0
    )
    , SHINY_SOUND_LOW_FREQUENCY(
        "<b>Shiny Sound Low Frequency (Hz):</b><br>High pass filter frequency for shiny sound.",
        LockMode::LOCK_WHILE_RUNNING,
        1000, 0, 48000
    )
{
    PA_ADD_STATIC(m_general);

    PA_ADD_STATIC(m_menu_navigation);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY0);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH0);
    PA_ADD_OPTION(START_GAME_WAIT1);
    PA_ADD_OPTION(ENTER_GAME_MASH0);
    PA_ADD_OPTION(ENTER_GAME_WAIT0);

    PA_ADD_STATIC(m_shiny_audio_settings);
    PA_ADD_OPTION(SHINY_SOUND_THRESHOLD);
    PA_ADD_OPTION(SHINY_SOUND_LOW_FREQUENCY);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonLGPE:GlobalSettings",
        STRING_POKEMON + " LGPE", "Game Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonLGPE/PokemonSettings.md",
        "Global " + STRING_POKEMON + " Let's Go Settings"
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






