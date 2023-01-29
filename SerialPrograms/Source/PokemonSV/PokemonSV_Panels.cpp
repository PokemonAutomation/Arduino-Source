/*  Pokemon SV Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonSV_Panels.h"

#include "PokemonSV_Settings.h"

#include "Programs/General/PokemonSV_MassPurchase.h"
#include "Programs/General/PokemonSV_MassRelease.h"
#include "Programs/General/PokemonSV_AutonomousBallThrower.h"

#include "Programs/General/PokemonSV_LPFarmer.h"
#include "Programs/General/PokemonSV_GimmighoulChestFarmer.h"
#include "Programs/General/PokemonSV_GimmighoulRoamingFarmer.h"
#include "Programs/General/PokemonSV_AuctionFarmer.h"
#include "Programs/General/PokemonSV_ESPTraining.h"
#include "Programs/Trading/PokemonSV_SelfBoxTrade.h"
#include "Programs/General/PokemonSV_StatsReset.h"

#include "Programs/Eggs/PokemonSV_EggFetcher.h"
#include "Programs/Eggs/PokemonSV_EggHatcher.h"
#include "Programs/Eggs/PokemonSV_EggAutonomous.h"

#include "Programs/TeraRaids/PokemonSV_AutoHost.h"
#include "Programs/TeraRaids/PokemonSV_TeraSelfFarmer.h"
#include "Programs/TeraRaids/PokemonSV_TeraMultiFarmer.h"

#include "Programs/FastCodeEntry/PokemonSV_FastCodeEntry.h"
#include "Programs/FastCodeEntry/PokemonSV_ClipboardFastCodeEntry.h"
#include "Programs/FastCodeEntry/PokemonSV_VideoFastCodeEntry.h"

#include "Programs/Glitches/PokemonSV_RideCloner-1.0.1.h"
#include "Programs/Glitches/PokemonSV_CloneItems-1.0.1.h"

#ifdef PA_OFFICIAL
#include "../../Internal/SerialPrograms/NintendoSwitch_TestPrograms.h"
#endif

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
    ret.emplace_back(make_single_switch_program<MassPurchase_Descriptor, MassPurchase>());
    ret.emplace_back(make_single_switch_program<MassRelease_Descriptor, MassRelease>());
    ret.emplace_back(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());
    ret.emplace_back(make_single_switch_program<StatsReset_Descriptor, StatsReset>());

//    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<LPFarmer_Descriptor, LPFarmer>());
    ret.emplace_back(make_single_switch_program<GimmighoulRoamingFarmer_Descriptor, GimmighoulRoamingFarmer>());
    ret.emplace_back(make_single_switch_program<GimmighoulChestFarmer_Descriptor, GimmighoulChestFarmer>());
    ret.emplace_back(make_single_switch_program<AuctionFarmer_Descriptor, AuctionFarmer>());
    ret.emplace_back(make_single_switch_program<ESPTraining_Descriptor, ESPTraining>());

    ret.emplace_back("---- Eggs ----");
    ret.emplace_back(make_single_switch_program<EggFetcher_Descriptor, EggFetcher>());
    ret.emplace_back(make_single_switch_program<EggHatcher_Descriptor, EggHatcher>());
    ret.emplace_back(make_single_switch_program<EggAutonomous_Descriptor, EggAutonomous>());

    ret.emplace_back("---- Tera Raids ----");
    ret.emplace_back(make_single_switch_program<AutoHost_Descriptor, AutoHost>());
    ret.emplace_back(make_single_switch_program<TeraSelfFarmer_Descriptor, TeraSelfFarmer>());
    ret.emplace_back(make_multi_switch_program<TeraMultiFarmer_Descriptor, TeraMultiFarmer>());

    ret.emplace_back("---- Fast Code Entry ----");
    ret.emplace_back(make_multi_switch_program<FastCodeEntry_Descriptor, FastCodeEntry>());
    ret.emplace_back(make_multi_switch_program<ClipboardFastCodeEntry_Descriptor, ClipboardFastCodeEntry>());
    ret.emplace_back(make_multi_switch_program<VideoFastCodeEntry_Descriptor, VideoFastCodeEntry>());

    ret.emplace_back("---- Glitches ----");
    ret.emplace_back(make_single_switch_program<RideCloner101_Descriptor, RideCloner101>());
    ret.emplace_back(make_single_switch_program<CloneItems101_Descriptor, CloneItems101>());

#ifdef PA_OFFICIAL
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Research ----");
        add_panels(ret);
    }
#endif

    return ret;
}




}
}
}
