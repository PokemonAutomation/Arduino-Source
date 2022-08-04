/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "PokemonLA_Panels.h"

#include "PokemonLA_Settings.h"

#include "Programs/PokemonLA_BraviaryHeightGlitch.h"
#include "Programs/PokemonLA_DistortionWaiter.h"
#include "Programs/PokemonLA_OutbreakFinder.h"
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
    add_program<BraviaryHeightGlitch_Descriptor, BraviaryHeightGlitch>();
    add_program<DistortionWaiter_Descriptor, DistortionWaiter>();
    add_program<OutbreakFinder_Descriptor, OutbreakFinder>();
    add_program<ClothingBuyer_Descriptor, ClothingBuyer>();
    add_program<SkipToFullMoon_Descriptor, SkipToFullMoon>();
    add_program<ApplyGrits_Descriptor, ApplyGrits>();
    add_program<PokedexTasksReader_Descriptor, PokedexTasksReader>();
    add_program<RamanasCombeeFinder_Descriptor, RamanasCombeeFinder>();

    add_divider("---- Trading ----");
    add_program<SelfBoxTrade_Descriptor, SelfBoxTrade>();
    add_program<SelfTouchTrade_Descriptor, SelfTouchTrade>();

    add_divider("---- Farming ----");
    add_program<NuggetFarmerHighlands_Descriptor, NuggetFarmerHighlands>();
    add_program<IngoBattleGrinder_Descriptor, IngoBattleGrinder>();
    add_program<IngoMoveGrinder_Descriptor, IngoMoveGrinder>();
    add_program<MagikarpMoveGrinder_Descriptor, MagikarpMoveGrinder>();
    add_program<TenacityCandyFarmer_Descriptor, TenacityCandyFarmer>();
    add_program<LeapGrinder_Descriptor, LeapGrinder>();

    add_divider("---- Shiny Hunting ----");
//    add_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>();
    add_program<CrobatFinder_Descriptor, CrobatFinder>();
    add_program<GalladeFinder_Descriptor, GalladeFinder>();
    add_program<FroslassFinder_Descriptor, FroslassFinder>();
    add_program<BurmyFinder_Descriptor, BurmyFinder>();
    add_program<UnownFinder_Descriptor, UnownFinder>();
    add_program<ShinyHuntFlagPin_Descriptor, ShinyHuntFlagPin>();
    add_program<PostMMOSpawnReset_Descriptor, PostMMOSpawnReset>();
    add_program<ShinyHuntCustomPath_Descriptor, ShinyHuntCustomPath>();
    add_program<AutoMultiSpawn_Descriptor, AutoMultiSpawn>();

    if (PreloadSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_program<MountDetectionTest_Descriptor, MountDetectionTest>();
        add_program<OverworldWatcher_Descriptor, OverworldWatcher>();
        add_program<SoundListener_Descriptor, SoundListener>();
        add_program<FlagNavigationTest_Descriptor, FlagNavigationTest>();
    }

    finish_panel_setup();
}




}
}
}
