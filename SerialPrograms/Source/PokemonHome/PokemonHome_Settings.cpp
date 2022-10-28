/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/NintendoSwitch/NintendoSwitch_ControllerDefs.h"
#include "CommonFramework/Globals.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonHome_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{
    using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockWhileRunning::LOCKED)
    // : m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    // // , START_GAME_MASH(
    // //     "<b>1. Start Game Mash:</b><br>Mash A for this long to start the game.",
    // //     TICKS_PER_SECOND,
    // //     "2 * TICKS_PER_SECOND"
    // // )
    // // , START_GAME_WAIT(
    // //     "<b>2. Start Game Wait:</b><br>Wait this long for the game to load.",
    // //     TICKS_PER_SECOND,
    // //     "40 * TICKS_PER_SECOND"
    // // )
    // // , ENTER_GAME_MASH(
    // //     "<b>3. Enter Game Mash:</b><br>Mash A for this long to enter the game.",
    // //     TICKS_PER_SECOND,
    // //     "5 * TICKS_PER_SECOND"
    // // )
    // // , ENTER_GAME_WAIT(
    // //     "<b>4. Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
    // //     TICKS_PER_SECOND,
    // //     "40 * TICKS_PER_SECOND"
    // // )
{
    // PA_ADD_STATIC(m_start_game_timings);
    // PA_ADD_OPTION(START_GAME_MASH);
    // PA_ADD_OPTION(START_GAME_WAIT);
    // PA_ADD_OPTION(ENTER_GAME_MASH);
    // PA_ADD_OPTION(ENTER_GAME_WAIT);
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonHome:GlobalSettings",
        STRING_POKEMON + " Home", STRING_POKEMON + " Settings",
        "ComputerControl/blob/master/Wiki/Programs/PokemonHome/PokemonSettings.md",
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






