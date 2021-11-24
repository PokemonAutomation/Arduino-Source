/*  Pokemon BDSP Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Globals.h"
#include "PokemonBDSP_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : m_menu_navigation("<font size=4><b>Menu Navigation Timings:</b></font>")
    , OVERWORLD_TO_MENU_DELAY(
        "<b>Overworld to Menu Delay:</b><br>Delay to bring up the menu when pressing X in the overworld.",
        "120"
    )
    , MENU_TO_OVERWORLD_DELAY(
        "<b>Menu to Overworld Delay:</b><br>Delay to go from menu back to overworld.",
        "250"
    )
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
        "35 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_MASH(
        "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
        "5 * TICKS_PER_SECOND"
    )
    , ENTER_GAME_WAIT(
        "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        "25 * TICKS_PER_SECOND"
    )
{
    PA_ADD_OPTION(GAME_TO_HOME_DELAY);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        QColor(),
        "PokemonBDSP:GlobalSettings",
        STRING_POKEMON + " Settings",
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






