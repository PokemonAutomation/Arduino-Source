/*  Pokemon Legends Arceus Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "PokemonLA_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
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
        "125"
    )
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_MASH(
        "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
        "2 * TICKS_PER_SECOND"
    )
    , START_GAME_WAIT(
        "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
        "25 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_MASH(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        "5 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        "15 * TICKS_PER_SECOND"
    )
    , SHINY_SHOUND_THRESHOLD0(
        "<b>Shiny Sound Threshold:</b><br>The shiny sound detection threshold.",
        0.85, 0, 1.0
    )
    , SHINY_SHOUND_LOW_FREQUENCY(
        "<b>Shiny Sound Low Frequency (Hz):</b><br>Low pass filter frequency for shiny sound.",
        5000, 0, 48000
    )
{
    PA_ADD_OPTION(m_menu_navigation);
//    PA_ADD_OPTION(OVERWORLD_TO_MENU_DELAY);
//    PA_ADD_OPTION(MENU_TO_OVERWORLD_DELAY);
    PA_ADD_OPTION(GAME_TO_HOME_DELAY);

    PA_ADD_OPTION(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_MASH);
    PA_ADD_OPTION(START_GAME_WAIT);
    PA_ADD_OPTION(ENTER_GAME_MASH);
    PA_ADD_OPTION(ENTER_GAME_WAIT);
    PA_ADD_OPTION(SHINY_SHOUND_THRESHOLD0);
    PA_ADD_OPTION(SHINY_SHOUND_LOW_FREQUENCY);
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






