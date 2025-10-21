/*  Pokemon LZA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_Panels.h"

#include "PokemonLZA_Settings.h"

#include "Programs/PokemonLZA_RestaurantFarmer.h"
#include "Programs/PokemonLZA_ClothingBuyer.h"
#include "Programs/TestPrograms/PokemonLZA_OverworldWatcher.h"

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

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<RestaurantFarmer_Descriptor, RestaurantFarmer>());
    ret.emplace_back(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());


    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<OverworldWatcher_Descriptor, OverworldWatcher>());
    }
    return ret;
}



}
}
}
