/*  Pokemon SV Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

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
    : BatchOption(LockMode::LOCK_WHILE_RUNNING)
{
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






