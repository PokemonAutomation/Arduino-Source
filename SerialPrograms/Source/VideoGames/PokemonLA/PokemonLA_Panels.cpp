/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/StaticGlobals.h"
#include "CommonFramework/GlobalAutoPaths.h"
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
    : PanelListDescriptor(
        Pokemon::STRING_POKEMON + " Legends: Arceus",
        RESOURCE_PATH() + "CategoryIcons/PokemonLA.png"
    )
{}

std::vector<PanelEntry> PanelListFactory::make_panels() const{
    std::vector<PanelEntry> ret;

    ret.emplace_back("---- Settings ----");
    ret.emplace_back(make_OptionsPanel<GameSettingsPanel>());

    ret.emplace_back("---- General ----");
    ret.emplace_back(make_SingleSwitchProgram<BraviaryHeightGlitch>());
    ret.emplace_back(make_SingleSwitchProgram<DistortionWaiter>());
    ret.emplace_back(make_SingleSwitchProgram<OutbreakFinder>());
    ret.emplace_back(make_SingleSwitchProgram<ClothingBuyer>());
    ret.emplace_back(make_SingleSwitchProgram<SkipToFullMoon>());
    ret.emplace_back(make_SingleSwitchProgram<ApplyGrits>());
    ret.emplace_back(make_SingleSwitchProgram<PokedexTasksReader>());
    ret.emplace_back(make_SingleSwitchProgram<RamanasCombeeFinder>());

    ret.emplace_back("---- Trading ----");
    ret.emplace_back(make_MultiSwitchProgram<SelfBoxTrade>());
    ret.emplace_back(make_MultiSwitchProgram<SelfTouchTrade>());

    ret.emplace_back("---- Farming ----");
    ret.emplace_back(make_SingleSwitchProgram<NuggetFarmerHighlands>());
    ret.emplace_back(make_SingleSwitchProgram<IngoBattleGrinder>());
    ret.emplace_back(make_SingleSwitchProgram<IngoMoveGrinder>());
    ret.emplace_back(make_SingleSwitchProgram<MagikarpMoveGrinder>());
    ret.emplace_back(make_SingleSwitchProgram<TenacityCandyFarmer>());
    ret.emplace_back(make_SingleSwitchProgram<LeapGrinder>());

    ret.emplace_back("---- Shiny Hunting ----");
    ret.emplace_back(make_SingleSwitchProgram<CrobatFinder>());
    ret.emplace_back(make_SingleSwitchProgram<GalladeFinder>());
    ret.emplace_back(make_SingleSwitchProgram<FroslassFinder>());
    ret.emplace_back(make_SingleSwitchProgram<BurmyFinder>());
    ret.emplace_back(make_SingleSwitchProgram<UnownFinder>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntFlagPin>());
    ret.emplace_back(make_SingleSwitchProgram<PostMMOSpawnReset>());
    ret.emplace_back(make_SingleSwitchProgram<ShinyHuntCustomPath>());

    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Untested/Beta/WIP ----");
        ret.emplace_back(make_SingleSwitchProgram<AutoMultiSpawn>());
    }
    if (STATIC_GLOBALS.DEVELOPER_MODE){
        ret.emplace_back("---- Machine Learning ----");
        ret.emplace_back(make_SingleSwitchProgram<GeneratePokemonImageTrainingData>());

        ret.emplace_back("---- Developer Tools ----");
        ret.emplace_back(make_SingleSwitchProgram<MountDetectionTest>());
        ret.emplace_back(make_SingleSwitchProgram<OverworldWatcher>());
        ret.emplace_back(make_SingleSwitchProgram<SoundListener>());
        ret.emplace_back(make_SingleSwitchProgram<FlagNavigationTest>());
    }

    return ret;
}




}
}
}
