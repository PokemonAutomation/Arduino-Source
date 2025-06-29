/*  Pokemon LGPE Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLGPE_Panels.h"
#include "PokemonLGPE_Settings.h"

#include "Programs/Farming/PokemonLGPE_DailyItemFarmer.h"
#include "Programs/ShinyHunting/PokemonLGPE_AlolanTrade.h"
#include "Programs/ShinyHunting/PokemonLGPE_FossilRevival.h"
#include "Programs/ShinyHunting/PokemonLGPE_GiftReset.h"
#include "Programs/ShinyHunting/PokemonLGPE_LegendaryReset.h"

#include "Programs/TestPrograms/PokemonLGPE_SoundListener.h"

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

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<DailyItemFarmer_Descriptor, DailyItemFarmer>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<AlolanTrade_Descriptor, AlolanTrade>());
    ret.emplace_back(make_single_switch_program<FossilRevival_Descriptor, FossilRevival>());
    ret.emplace_back(make_single_switch_program<GiftReset_Descriptor, GiftReset>());
    ret.emplace_back(make_single_switch_program<LegendaryReset_Descriptor, LegendaryReset>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }

    return ret;
}




}
}
}
