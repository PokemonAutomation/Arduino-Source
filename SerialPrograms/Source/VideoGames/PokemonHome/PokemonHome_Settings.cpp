/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */
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
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
{
}





GameSettings_Descriptor::GameSettings_Descriptor()
    : OptionsPanelDescriptor(
        "PokemonHome:GlobalSettings",
        STRING_POKEMON + " Home", STRING_POKEMON + " Settings",
        "Programs/PokemonHome/PokemonSettings.html",
        "Global " + STRING_POKEMON + " Settings",
        PanelQualityStanding::OFFICIAL
    )
{}



GameSettingsPanel::GameSettingsPanel()
    : settings(GameSettings::instance())
{
    PA_ADD_OPTION(settings);
}





}
}
}






