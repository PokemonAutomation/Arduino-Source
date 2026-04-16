/*  Pokemon Pokopia
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonPokopia_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{

using namespace Pokemon;



GameSettings& GameSettings::instance(){
    static GameSettings settings;
    return settings;
}
GameSettings::GameSettings()
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
    , m_general("<font size=4><b>General Settings:</b></font>")
    , m_advanced_options(
        "<font size=4><b>Advanced Options:</b> You should not need to touch anything below here.</font>"
	)
	,GAME_TO_HOME_DELAY0(
        "<b>Game to Home Delay:</b><br>Delay from pressing home to entering the the Switch home menu.",
        LockMode::LOCK_WHILE_RUNNING,
        "1000 ms"
    )
    
{
    PA_ADD_STATIC(m_general);
    PA_ADD_STATIC(m_advanced_options);
	PA_ADD_OPTION(GAME_TO_HOME_DELAY0);
}




GameSettings_Descriptor::GameSettings_Descriptor()
    : PanelDescriptor(
        Color(),
        "PokemonPokopia:GlobalSettings",
        STRING_POKEMON + " Pokopia", "Game Settings",
        "Programs/PokemonPokopia/PokemonSettings.html",
        "Global " + STRING_POKEMON + " Pokopia Settings"
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






