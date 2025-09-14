/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Pokemon/Pokemon_Strings.h"
#include "PokemonLA_Panels.h"

#include "PokemonLA_Settings.h"

#include "Programs/General/PokemonLA_BraviaryHeightGlitch.h"
#include "Programs/General/PokemonLA_DistortionWaiter.h"
#include "Programs/General/PokemonLA_OutbreakFinder.h"
#include "Programs/General/PokemonLA_ClothingBuyer.h"
#include "Programs/General/PokemonLA_SkipToFullMoon.h"
#include "Programs/General/PokemonLA_PokedexTasksReader.h"
#include "Programs/General/PokemonLA_RamanasIslandCombee.h"
#include "Programs/General/PokemonLA_ApplyGrits.h"

#include "Programs/Trading/PokemonLA_SelfBoxTrade.h"
#include "Programs/Trading/PokemonLA_SelfTouchTrade.h"

#include "Programs/Farming/PokemonLA_IngoBattleGrinder.h"
#include "Programs/Farming/PokemonLA_IngoMoveGrinder.h"
#include "Programs/Farming/PokemonLA_MagikarpMoveGrinder.h"
#include "Programs/Farming/PokemonLA_NuggetFarmerHighlands.h"
#include "Programs/Farming/PokemonLA_TenacityCandyFarmer.h"
#include "Programs/Farming/PokemonLA_LeapGrinder.h"

//#include "Programs/ShinyHunting/PokemonLA_ShinyHunt-LakeTrio.h"
#include "Programs/ShinyHunting/PokemonLA_AutoMultiSpawn.h"
#include "Programs/ShinyHunting/PokemonLA_GalladeFinder.h"
#include "Programs/ShinyHunting/PokemonLA_CrobatFinder.h"
#include "Programs/ShinyHunting/PokemonLA_FroslassFinder.h"
#include "Programs/ShinyHunting/PokemonLA_BurmyFinder.h"
#include "Programs/ShinyHunting/PokemonLA_UnownFinder.h"
#include "Programs/ShinyHunting/PokemonLA_ShinyHunt-FlagPin.h"
#include "Programs/ShinyHunting/PokemonLA_PostMMOSpawnReset.h"
#include "Programs/ShinyHunting/PokemonLA_ShinyHunt-CustomPath.h"

#include "Programs/TestPrograms/PokemonLA_MountDetectionTest.h"
#include "Programs/TestPrograms/PokemonLA_OverworldWatcher.h"
#include "Programs/TestPrograms/PokemonLA_FlagNavigationTest.h"
#include "Programs/TestPrograms/PokemonLA_SoundListener.h"

#include "Programs/ML/PokemonLA_GeneratePokemonImageTrainingData.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



PanelListFactory::PanelListFactory()
    : PanelListDescriptor(Pokemon::STRING_POKEMON + " Legends Arceus")
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_settings<GameSettings_Descriptor, GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_single_switch_program<BraviaryHeightGlitch_Descriptor, BraviaryHeightGlitch>());
    ret.emplace_back(make_single_switch_program<DistortionWaiter_Descriptor, DistortionWaiter>());
    ret.emplace_back(make_single_switch_program<OutbreakFinder_Descriptor, OutbreakFinder>());
    ret.emplace_back(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());
    ret.emplace_back(make_single_switch_program<SkipToFullMoon_Descriptor, SkipToFullMoon>());
    ret.emplace_back(make_single_switch_program<ApplyGrits_Descriptor, ApplyGrits>());
    ret.emplace_back(make_single_switch_program<PokedexTasksReader_Descriptor, PokedexTasksReader>());
    ret.emplace_back(make_single_switch_program<RamanasCombeeFinder_Descriptor, RamanasCombeeFinder>());

    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());
    ret.emplace_back(make_multi_switch_program<SelfTouchTrade_Descriptor, SelfTouchTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_single_switch_program<NuggetFarmerHighlands_Descriptor, NuggetFarmerHighlands>());
    ret.emplace_back(make_single_switch_program<IngoBattleGrinder_Descriptor, IngoBattleGrinder>());
    ret.emplace_back(make_single_switch_program<IngoMoveGrinder_Descriptor, IngoMoveGrinder>());
    ret.emplace_back(make_single_switch_program<MagikarpMoveGrinder_Descriptor, MagikarpMoveGrinder>());
    ret.emplace_back(make_single_switch_program<TenacityCandyFarmer_Descriptor, TenacityCandyFarmer>());
    ret.emplace_back(make_single_switch_program<LeapGrinder_Descriptor, LeapGrinder>());

    ret.emplace_back("---- Shiny Hunting ----");
//    ret.emplace_back(make_single_switch_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>());
    ret.emplace_back(make_single_switch_program<CrobatFinder_Descriptor, CrobatFinder>());
    ret.emplace_back(make_single_switch_program<GalladeFinder_Descriptor, GalladeFinder>());
    ret.emplace_back(make_single_switch_program<FroslassFinder_Descriptor, FroslassFinder>());
    ret.emplace_back(make_single_switch_program<BurmyFinder_Descriptor, BurmyFinder>());
    ret.emplace_back(make_single_switch_program<UnownFinder_Descriptor, UnownFinder>());
    ret.emplace_back(make_single_switch_program<ShinyHuntFlagPin_Descriptor, ShinyHuntFlagPin>());
    ret.emplace_back(make_single_switch_program<PostMMOSpawnReset_Descriptor, PostMMOSpawnReset>());
    ret.emplace_back(make_single_switch_program<ShinyHuntCustomPath_Descriptor, ShinyHuntCustomPath>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_single_switch_program<AutoMultiSpawn_Descriptor, AutoMultiSpawn>());
    }
    if (PreloadSettings::instance().DEVELOPER_MODE){
        ret.emplace_back("---- Machine Learning ----");
        ret.emplace_back(make_single_switch_program<GeneratePokemonImageTrainingData_Descriptor, GeneratePokemonImageTrainingData>());

        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_single_switch_program<MountDetectionTest_Descriptor, MountDetectionTest>());
        ret.emplace_back(make_single_switch_program<OverworldWatcher_Descriptor, OverworldWatcher>());
        ret.emplace_back(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
        ret.emplace_back(make_single_switch_program<FlagNavigationTest_Descriptor, FlagNavigationTest>());
    }

    return ret;
}




}
}
}
