/*  Pokemon LZA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLZA_Panels.h"

#include "PokemonLZA_Settings.h"

//  General
#include "Programs/PokemonLZA_BoxSorter.h"
#include "Programs/PokemonLZA_ClothingBuyer.h"
#include "Programs/PokemonLZA_StallBuyer.h"
#include "Programs/PokemonLZA_PostKillCatcher.h"
#include "Programs/PokemonLZA_TurboMacro.h"

//  Trading
#include "Programs/Trading/PokemonLZA_SelfBoxTrade.h"

//  Farming
#include "Programs/Farming/PokemonLZA_DonutMaker.h"
#include "Programs/Farming/PokemonLZA_RestaurantFarmer.h"
#include "Programs/Farming/PokemonLZA_MegaShardFarmer.h"
#include "Programs/Farming/PokemonLZA_JacintheInfiniteFarmer.h"
#include "Programs/Farming/PokemonLZA_FriendshipFarmer.h"
#include "Programs/Farming/PokemonLZA_InPlaceCatcher.h"
#include "Programs/Farming/PokemonLZA_HyperspaceRewardReset.h"
#include "Programs/Farming/PokemonLZA_WigglytuffFarmer.h"

//  Shiny Hunting
#include "Programs/ShinyHunting/PokemonLZA_AutoFossil.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_BenchSit.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_OverworldReset.h"
#include "Programs/ShinyHunting/PokemonLZA_BeldumHunter.h"
#include "Programs/ShinyHunting/PokemonLZA_WildZoneEntrance.h"
#include "Programs/ShinyHunting/PokemonLZA_WildZoneCafe.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_FlySpotReset.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_HyperspaceHunter.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_HyperspaceLegendary.h"
#include "Programs/ShinyHunting/PokemonLZA_ShuttleRun.h"
#include "Programs/ShinyHunting/PokemonLZA_SewerHunter.h"
#include "Programs/ShinyHunting/PokemonLZA_ShinyHunt_HelioptileHunter.h"

//  Non-Shiny Hunting
#include "Programs/NonShinyHunting/PokemonLZA_StatsReset.h"
#include "Programs/NonShinyHunting/PokemonLZA_WeatherFinder.h"

//  Developer
#include "Programs/TestPrograms/PokemonLZA_OverworldWatcher.h"
#include "Programs/TestPrograms/PokemonLZA_MoveBoxArrow.h"
#include "Programs/TestPrograms/PokemonLZA_TestBoxCellInfo.h"
#include "InferenceTraining/PokemonLZA_GenerateLocationNameOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Legends: Z-A",
        RESOURCE_PATH() + "CategoryIcons/PokemonLZA.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_SingleSwitchProgram<ClothingBuyer>());
    ret.emplace_back(make_SingleSwitchProgram<StallBuyer>());
    ret.emplace_back(make_MultiSwitchProgram<SelfBoxTrade>());
    ret.emplace_back(make_SingleSwitchProgram<PostKillCatcher>());
    ret.emplace_back(make_SingleSwitchProgram<BoxSorter>());
    ret.emplace_back(make_SingleSwitchProgram<WeatherFinder>());
    ret.emplace_back(make_SingleSwitchProgram<HyperspaceRewardReset>());
    ret.emplace_back(make_SingleSwitchProgram<DonutMaker>());
    if (IS_BETA_VERSION){
        ret.emplace_back(make_SingleSwitchProgram<LZA_TurboMacro>());
    }

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_SingleSwitchProgram<RestaurantFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<MegaShardFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<JacintheInfiniteFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<FriendshipFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<InPlaceCatcher>());
    ret.emplace_back(make_SingleSwitchProgram<WigglytuffFarmer>());
    if (IS_BETA_VERSION){
    }

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<AutoFossil>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_BenchSit>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_OverworldReset>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_WildZoneEntrance>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_WildZoneCafe>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_FlySpotReset>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_SewerHunter>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_HelioptileHunter>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_HyperspaceLegendary>());
    if (IS_BETA_VERSION){
        ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_ShuttleRun>());
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back(make_SingleSwitchProgram<ShinyHunt_HyperspaceHunter>());
        ret.emplace_back(make_SingleSwitchProgram<BeldumHunter>());
    }

//    ret.emplace_back("---- Non-Shiny Hunting ----");
    if (IS_BETA_VERSION){
    }

    ret.emplace_back("---- Public Betas ----");
    ret.emplace_back(make_SingleSwitchProgram<StatsReset>());

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<OverworldWatcher>());
        ret.emplace_back(make_SingleSwitchProgram<MoveBoxArrow>());
        ret.emplace_back(make_SingleSwitchProgram<TestBoxCellInfo>());
        ret.emplace_back(make_SingleSwitchProgram<GenerateLocationNameOCR>());
    }
    return ret;
}



}
}
}
