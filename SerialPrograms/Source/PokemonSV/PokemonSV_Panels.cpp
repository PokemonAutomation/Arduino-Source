/*  Pokemon SV Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_Panels.h"

#include "PokemonSV_Settings.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Scarlet and Violet", false)
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
//    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

//    ret.emplace_back("---- General ----");

//    ret.emplace_back("---- Trading ----");

//    ret.emplace_back("---- Farming ----");

//    ret.emplace_back("---- Shiny Hunting ----");

    if (PreloadSettings::instance().DEVELOPER_MODE){
//        ret.emplace_back("---- Developer Tools ----");
    }

    return ret;
}




}
}
}
