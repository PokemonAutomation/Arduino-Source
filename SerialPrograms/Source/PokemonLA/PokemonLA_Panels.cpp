/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
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

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


Panels::Panels(QTabWidget& parent, PanelHolder& holder)
    : PanelList(parent, "LA", holder)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- General ----");
    add_panel(make_single_switch_program<BraviaryHeightGlitch_Descriptor, BraviaryHeightGlitch>());
    add_panel(make_single_switch_program<DistortionWaiter_Descriptor, DistortionWaiter>());
    add_panel(make_single_switch_program<OutbreakFinder_Descriptor, OutbreakFinder>());
    add_panel(make_single_switch_program<ClothingBuyer_Descriptor, ClothingBuyer>());
    add_panel(make_single_switch_program<SkipToFullMoon_Descriptor, SkipToFullMoon>());
    add_panel(make_single_switch_program<ApplyGrits_Descriptor, ApplyGrits>());
    add_panel(make_single_switch_program<PokedexTasksReader_Descriptor, PokedexTasksReader>());
    add_panel(make_single_switch_program<RamanasCombeeFinder_Descriptor, RamanasCombeeFinder>());

    add_divider("---- Trading ----");
    add_panel(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());
    add_panel(make_multi_switch_program<SelfTouchTrade_Descriptor, SelfTouchTrade>());

    add_divider("---- Farming ----");
    add_panel(make_single_switch_program<NuggetFarmerHighlands_Descriptor, NuggetFarmerHighlands>());
    add_panel(make_single_switch_program<IngoBattleGrinder_Descriptor, IngoBattleGrinder>());
    add_panel(make_single_switch_program<IngoMoveGrinder_Descriptor, IngoMoveGrinder>());
    add_panel(make_single_switch_program<MagikarpMoveGrinder_Descriptor, MagikarpMoveGrinder>());
    add_panel(make_single_switch_program<TenacityCandyFarmer_Descriptor, TenacityCandyFarmer>());
    add_panel(make_single_switch_program<LeapGrinder_Descriptor, LeapGrinder>());

    add_divider("---- Shiny Hunting ----");
//    add_panel(make_single_switch_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>());
    add_panel(make_single_switch_program<CrobatFinder_Descriptor, CrobatFinder>());
    add_panel(make_single_switch_program<GalladeFinder_Descriptor, GalladeFinder>());
    add_panel(make_single_switch_program<FroslassFinder_Descriptor, FroslassFinder>());
    add_panel(make_single_switch_program<BurmyFinder_Descriptor, BurmyFinder>());
    add_panel(make_single_switch_program<UnownFinder_Descriptor, UnownFinder>());
    add_panel(make_single_switch_program<ShinyHuntFlagPin_Descriptor, ShinyHuntFlagPin>());
    add_panel(make_single_switch_program<PostMMOSpawnReset_Descriptor, PostMMOSpawnReset>());
    add_panel(make_single_switch_program<ShinyHuntCustomPath_Descriptor, ShinyHuntCustomPath>());
    if (PreloadSettings::instance().DEVELOPER_MODE){
        add_panel(make_single_switch_program<AutoMultiSpawn_Descriptor, AutoMultiSpawn>());
    }

    if (PreloadSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_panel(make_single_switch_program<MountDetectionTest_Descriptor, MountDetectionTest>());
        add_panel(make_single_switch_program<OverworldWatcher_Descriptor, OverworldWatcher>());
        add_panel(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
        add_panel(make_single_switch_program<FlagNavigationTest_Descriptor, FlagNavigationTest>());
    }

    finish_panel_setup();
}




}
}
}
