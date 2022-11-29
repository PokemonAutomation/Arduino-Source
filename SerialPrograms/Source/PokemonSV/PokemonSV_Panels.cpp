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
#include "Programs/General/PokemonSV_AutonomousBallThrower.h"
#include "Programs/Trading/PokemonSV_SelfBoxTrade.h"

#include "Programs/TeraRaids/PokemonSV_TeraSelfFarmer.h"

#include "Programs/PokemonSV_FastCodeEntry.h"
#include "Programs/PokemonSV_AutoHost.h"

#include "Programs/Glitches/PokemonSV_RideCloner-1.0.1.h"
#include "Programs/Glitches/PokemonSV_CloneItems-1.0.1.h"

#include "Programs/PokemonSV_EggFetcher.h"

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
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());
    }

//    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<TeraSelfFarmer_Descriptor, TeraSelfFarmer>());

    ret.emplace_back("---- Multiplayer ----");
    ret.emplace_back(make_multi_switch_program<FastCodeEntry_Descriptor, FastCodeEntry>());
    ret.emplace_back(make_single_switch_program<AutoHost_Descriptor, AutoHost>());

    ret.emplace_back("---- Glitches ----");
    ret.emplace_back(make_single_switch_program<RideCloner101_Descriptor, RideCloner101>());
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back(make_single_switch_program<CloneItems101_Descriptor, CloneItems101>());
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
