/*  Pokemon Legends ZA
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_general("<font size=4><b>General Settings:</b></font>")
    , m_start_game_timings("<font size=4><b>Start Game Timings:</b></font>")
    , START_GAME_WAIT(
        "<b>Start Game Wait:</b><br>Wait this long for the game to load.",
        LockMode::LOCK_WHILE_RUNNING,
        "40 s"
    )
    , ENTER_GAME_WAIT0(
        "<b>Enter Game Wait:</b><br>Wait this long for the game to enter the overworld.",
        LockMode::LOCK_WHILE_RUNNING,
        "60 s"
    )
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
    )
{
    PA_ADD_STATIC(m_general);

    PA_ADD_STATIC(m_start_game_timings);
    PA_ADD_OPTION(START_GAME_WAIT);
    PA_ADD_OPTION(ENTER_GAME_WAIT0);

    PA_ADD_STATIC(m_advanced_options);
}




GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonLZA:GlobalSettings",
        STRING_POKEMON + " LZA", "Game Settings",
        "Programs/PokemonLZA/PokemonSettings.html",
        "Global " + STRING_POKEMON + " Legends ZA Settings"
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






