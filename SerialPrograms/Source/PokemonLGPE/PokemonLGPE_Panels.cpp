/*  Pokemon LGPE Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLGPE_Panels.h"
#include "PokemonLGPE_Settings.h"

#include "Programs/ShinyHunting/PokemonLGPE_AlolanTrade.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Let's Go Pikachu and Eevee")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<AlolanTrade_Descriptor, AlolanTrade>());

    return ret;
}




}
}
}
