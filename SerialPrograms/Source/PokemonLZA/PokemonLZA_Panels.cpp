/*  Pokemon LZA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_Panels.h"

#include "PokemonLZA_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Legends ZA")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());


    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");

    }
    return ret;
}



}
}
}
