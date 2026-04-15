/*  Pokemon FRLG Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonPokopia_Panels.h"

#include "PokemonPokopia_Settings.h"

// General
#include "Programs/PokemonPokopia_CloudIslandReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonPokopia{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Pokopia")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    // ret.emplace_back("---- Farming ----");

    ret.emplace_back("---- General ----");
    if (IS_BETA_VERSION || PreloadSettings::instance().DEVELOPER_MODE){
        // Locking behind dev mode for now since this program is destructive and deletes your cloud island
        ret.emplace_back(make_single_switch_program<CloudIslandReset_Descriptor, CloudIslandReset>());
    }

    // if (PreloadSettings::instance().DEVELOPER_MODE){
    //     ret.emplace_back("---- Developer Tools ----");
    // }

    return ret;
}



}
}
}
