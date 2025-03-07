/*  Pokemon LGPE Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLGPE_Panels.h"



namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLGPE{

PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Let's Go Pikachu and Eevee")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    //ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting ----");
    //ret.emplace_back(make_single_switch_program<AudioStarterReset_Descriptor, AudioStarterReset>());

    return ret;
}




}
}
}
