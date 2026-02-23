/*  Pokemon FRLG Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonFRLG_Panels.h"

#include "PokemonFRLG_Settings.h"

#include "Programs/ShinyHunting/PokemonFRLG_GiftReset.h"
#include "Programs/ShinyHunting/PokemonFRLG_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonFRLG_PrizeCornerReset.h"
#include "Programs/TestPrograms/PokemonFRLG_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " FireRed and LeafGreen")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;
    
    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    //ret.emplace_back("---- General ----");

    ret.emplace_back("---- Shiny Hunting  ----");
    ret.emplace_back(make_single_switch_program<GiftReset_Descriptor, GiftReset>());
    ret.emplace_back(make_single_switch_program<LegendaryReset_Descriptor, LegendaryReset>());
    ret.emplace_back(make_single_switch_program<PrizeCornerReset_Descriptor, PrizeCornerReset>());
    

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }

    return ret;
}




}
}
}
