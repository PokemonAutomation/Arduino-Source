/*  Pokemon RSE Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Pokemon/Pokemon_Strings.h"
#include "PokemonRSE_Panels.h"

//#include "PokemonSV_Settings.h"

#include "Programs/ShinyHunting/PokemonRSE_StarterReset.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor("Pokemon Ruby, Sapphire, and Emerald")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    // ret.emplace_back("---- Settings ----"); TODO: Add device selection?
    // ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<StarterReset_Descriptor, StarterReset>());

    return ret;
}




}
}
}
