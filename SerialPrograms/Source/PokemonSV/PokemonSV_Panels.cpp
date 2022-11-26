/*  Pokemon SV Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_Panels.h"

#include "PokemonSV_Settings.h"

#include "Programs/PokemonSV_MassRelease.h"
#include "Programs/PokemonSV_SelfBoxTrade.h"
#include "Programs/PokemonSV_EggFetcher.h"
#include "Programs/PokemonSV_TeraSelfFarmer.h"
#include "Programs/PokemonSV_FastCodeEntry.h"
#include "Programs/PokemonSV_AutoHost.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Scarlet and Violet")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<MassRelease_Descriptor, MassRelease>());

//    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<TeraSelfFarmer_Descriptor, TeraSelfFarmer>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Multiplayer ----");
        ret.emplace_back(make_single_switch_program<FastCodeEntry_Descriptor, FastCodeEntry>());
        ret.emplace_back(make_single_switch_program<AutoHost_Descriptor, AutoHost>());
    }

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<EggFetcher_Descriptor, EggFetcher>());
    }

    return ret;
}




}
}
}
