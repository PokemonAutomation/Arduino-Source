/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Panels_PokemonLA.h"

#include "PokemonLA_Settings.h"

#include "Programs/PokemonLA_BraviaryHeightGlitch.h"
#include "Programs/PokemonLA_DistortionWaiter.h"
#include "Programs/PokemonLA_OutbreakFinder.h"

#include "Programs/Trading/PokemonLA_SelfBoxTrade.h"
#include "Programs/Trading/PokemonLA_SelfTouchTrade.h"

#include "Programs/Farming/PokemonLA_IngoBattleGrinder.h"
#include "Programs/Farming/PokemonLA_NuggetFarmerHighlands.h"

#include "Programs/ShinyHunting/PokemonLA_GalladeFinder.h"
#include "Programs/ShinyHunting/PokemonLA_CrobatFinder.h"
#include "Programs/ShinyHunting/PokemonLA_UnownFinder.h"
#include "Programs/ShinyHunting/PokemonLA_FroslassFinder.h"
#include "Programs/ShinyHunting/PokemonLA_ShinyHunt-FlagPin.h"
#include "Programs/ShinyHunting/PokemonLA_PostMMOSpawnReset.h"
//#include "Programs/ShinyHunting/PokemonLA_ShinyHunt-LakeTrio.h"

#include "Programs/TestPrograms/PokemonLA_MountDetectionTest.h"
#include "Programs/TestPrograms/PokemonLA_OverworldWatcher.h"
#include "Programs/TestPrograms/PokemonLA_ShinySoundListener.h"
#include "Programs/TestPrograms/PokemonLA_FlagNavigationTest.h"
#include "Programs/TestPrograms/PokemonLA_AlphaRoarListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "LA", listener)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- General ----");
    add_program<BraviaryHeightGlitch_Descriptor, BraviaryHeightGlitch>();
    add_program<DistortionWaiter_Descriptor, DistortionWaiter>();
    add_program<OutbreakFinder_Descriptor, OutbreakFinder>();

    add_divider("---- Trading ----");
    add_program<SelfBoxTrade_Descriptor, SelfBoxTrade>();
    add_program<SelfTouchTrade_Descriptor, SelfTouchTrade>();

    add_divider("---- Farming ----");
    add_program<NuggetFarmerHighlands_Descriptor, MoneyFarmerHighlands>();
    add_program<IngoBattleGrinder_Descriptor, IngoBattleGrinder>();

    add_divider("---- Shiny Hunting ----");
//    add_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>();
    add_program<CrobatFinder_Descriptor, CrobatFinder>();
    add_program<GalladeFinder_Descriptor, GalladeFinder>();
    add_program<UnownFinder_Descriptor, UnownFinder>();
    add_program<FroslassFinder_Descriptor, FroslassFinder>();
    add_program<ShinyHuntFlagPin_Descriptor, ShinyHuntFlagPin>();
    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_program<PostMMOSpawnReset_Descriptor, PostMMOSpawnReset>();
    }


    add_divider("---- Developer Tools ----");
    add_program<MountDetectionTest_Descriptor, MountDetectionTest>();
    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_program<OverworldWatcher_Descriptor, OverworldWatcher>();
        add_program<ShinySoundListener_Descriptor, ShinySoundListener>();
        add_program<AlphaRoarListener_Descriptor, AlphaRoarListener>();
        add_program<FlagNavigationTest_Descriptor, FlagNavigationTest>();
    }

    finish_panel_setup();
}




}
}
}
