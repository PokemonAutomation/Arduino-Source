/*  Pokemon LZA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_Panels.h"

#include "PokemonLZA_Settings.h"

//  General
#include "Programs/PokemonLZA_ClothingBuyer.h"
#include "Programs/PokemonLZA_StallBuyer.h"
#include "Programs/PokemonLZA_PostKillCatcher.h"

//  Trading
#include "Programs/Trading/PokemonLZA_SelfBoxTrade.h"

//  Farming
#include "Programs/Farming/PokemonLZA_RestaurantFarmer.h"
#include "Programs/Farming/PokemonLZA_MegaShardFarmer.h"
#include "Programs/Farming/PokemonLZA_JacintheInfiniteFarmer.h"
#include "Programs/Farming/PokemonLZA_FriendshipFarmer.h"
#include "Programs/Farming/PokemonLZA_InPlaceCatcher.h"

//  Shiny Hunting
#include "Programs/ShinyHunting/PokemonLZA_AutoFossil.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_BenchSit.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_OverworldReset.h"
#include "Programs/ShinyHunting/PokemonLZA_BeldumHunter.h"
#include "Programs/ShinyHunting/PokemonLZA_WildZoneEntrance.h"
#include "Programs/ShinyHunting/PokemonLZA_WildZoneCafe.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_FlySpotReset.h"
#include "Programs/ShinyHunting/PokemonLZA_ShuttleRun.h"
#include "Programs/ShinyHunting/PokemonLZA_SewerHunter.h"

//  Non-Shiny Hunting
#include "Programs/NonShinyHunting/PokemonLZA_StatsReset.h"

//  Developer
#include "Programs/TestPrograms/PokemonLZA_OverworldWatcher.h"
#include "Programs/TestPrograms/PokemonLZA_MoveBoxArrow.h"
#include "Programs/TestPrograms/PokemonLZA_TestBoxCellInfo.h"
#include "InferenceTraining/PokemonLZA_GenerateLocationNameOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Legends: Z-A")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());
    ret.emplace_back(make_single_switch_program<StallBuyer_Descriptor, StallBuyer>());
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());
    ret.emplace_back(make_single_switch_program<PostKillCatcher_Descriptor, PostKillCatcher>());
    if (IS_BETA_VERSION){
    }

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<RestaurantFarmer_Descriptor, RestaurantFarmer>());
    ret.emplace_back(make_single_switch_program<MegaShardFarmer_Descriptor, MegaShardFarmer>());
    ret.emplace_back(make_single_switch_program<JacintheInfiniteFarmer_Descriptor, JacintheInfiniteFarmer>());
    ret.emplace_back(make_single_switch_program<FriendshipFarmer_Descriptor, FriendshipFarmer>());
    if (IS_BETA_VERSION){
        ret.emplace_back(make_single_switch_program<InPlaceCatcher_Descriptor, InPlaceCatcher>());
    }

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_single_switch_program<AutoFossil_Descriptor, AutoFossil>());
    ret.emplace_back(make_single_switch_program<ShinyHunt_BenchSit_Descriptor, ShinyHunt_BenchSit>());
    ret.emplace_back(make_single_switch_program<ShinyHunt_OverworldReset_Descriptor, ShinyHunt_OverworldReset>());
    ret.emplace_back(make_single_switch_program<ShinyHunt_WildZoneEntrance_Descriptor, ShinyHunt_WildZoneEntrance>());
    if (IS_BETA_VERSION){
        ret.emplace_back(make_single_switch_program<ShinyHunt_WildZoneCafe_Descriptor, ShinyHunt_WildZoneCafe>());
        ret.emplace_back(make_single_switch_program<ShinyHunt_FlySpotReset_Descriptor, ShinyHunt_FlySpotReset>());
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back(make_single_switch_program<BeldumHunter_Descriptor, BeldumHunter>());
        ret.emplace_back(make_single_switch_program<ShinyHunt_ShuttleRun_Descriptor, ShinyHunt_ShuttleRun>());
        ret.emplace_back(make_single_switch_program<ShinyHunt_SewerHunter_Descriptor, ShinyHunt_SewerHunter>());
    }

//    ret.emplace_back("---- Non-Shiny Hunting ----");
    if (IS_BETA_VERSION){
    }

    ret.emplace_back("---- Public Betas ----");
    ret.emplace_back(make_single_switch_program<StatsReset_Descriptor, StatsReset>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<OverworldWatcher_Descriptor, OverworldWatcher>());
        ret.emplace_back(make_single_switch_program<MoveBoxArrow_Descriptor, MoveBoxArrow>());
        ret.emplace_back(make_single_switch_program<TestBoxCellInfo_Descriptor, TestBoxCellInfo>());
        ret.emplace_back(make_single_switch_program<GenerateLocationNameOCR_Descriptor, GenerateLocationNameOCR>());
    }
    return ret;
}



}
}
}
