/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "PokemonBDSP_Panels.h"

#include "PokemonBDSP_Settings.h"

#include "Programs/General/PokemonBDSP_MassRelease.h"
#include "Programs/General/PokemonBDSP_AutonomousBallThrower.h"

#include "Programs/Trading/PokemonBDSP_SelfBoxTrade.h"
#include "Programs/Trading/PokemonBDSP_SelfTouchTrade.h"

#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute210.h"
#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute212.h"
#include "Programs/Farming/PokemonBDSP_DoublesLeveling.h"
#include "Programs/Farming/PokemonBDSP_AmitySquarePickUpFarmer.h"
#include "Programs/Farming/PokemonBDSP_GiftBerryReset.h"

#include "Programs/ShinyHunting/PokemonBDSP_StarterReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Overworld.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Fishing.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Shaymin.h"

#include "Programs/Eggs/PokemonBDSP_EggFetcher.h"
#include "Programs/Eggs/PokemonBDSP_EggHatcher.h"
#include "Programs/Eggs/PokemonBDSP_EggAutonomous.h"

#include "Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.3.h"
#include "Programs/Glitches/PokemonBDSP_ActivateMenuGlitch-1.1.2.h"
#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy2.h"
//#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy.h"
//#include "Programs/Glitches/PokemonBDSP_CloneItemsMenuOverlap.h"

#include "Programs/TestPrograms/PokemonBDSP_ShinyEncounterTester.h"
#include "Programs/TestPrograms/PokemonBDSP_SoundListener.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


Panels::Panels(QTabWidget& parent, PanelHolder& holder)
    : PanelList(parent, "BD/SP", holder)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- General ----");
    add_panel(make_single_switch_program<MassRelease_Descriptor, MassRelease>());
    add_panel(make_single_switch_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>());

    add_divider("---- Trading ----");
    add_panel(make_multi_switch_program<SelfBoxTrade_Descriptor, SelfBoxTrade>());
    add_panel(make_multi_switch_program<SelfTouchTrade_Descriptor, SelfTouchTrade>());

    add_divider("---- Farming ----");
    add_panel(make_single_switch_program<MoneyFarmerRoute212_Descriptor, MoneyFarmerRoute212>());
    add_panel(make_single_switch_program<MoneyFarmerRoute210_Descriptor, MoneyFarmerRoute210>());
    add_panel(make_single_switch_program<DoublesLeveling_Descriptor, DoublesLeveling>());
    add_panel(make_single_switch_program<AmitySquarePickUpFarmer_Descriptor, AmitySquarePickUpFarmer>());
    add_panel(make_single_switch_program<GiftBerryReset_Descriptor, GiftBerryReset>());

    add_divider("---- Shiny Hunting ----");
    add_panel(make_single_switch_program<StarterReset_Descriptor, StarterReset>());
    add_panel(make_single_switch_program<LegendaryReset_Descriptor, LegendaryReset>());
    add_panel(make_single_switch_program<ShinyHuntOverworld_Descriptor, ShinyHuntOverworld>());
    add_panel(make_single_switch_program<ShinyHuntFishing_Descriptor, ShinyHuntFishing>());
    add_panel(make_single_switch_program<ShinyHuntShaymin_Descriptor, ShinyHuntShaymin>());

    add_divider("---- Eggs ----");
    add_panel(make_single_switch_program<EggFetcher_Descriptor, EggFetcher>());
    add_panel(make_single_switch_program<EggHatcher_Descriptor, EggHatcher>());
    add_panel(make_single_switch_program<EggAutonomous_Descriptor, EggAutonomous>());

    add_divider("---- Glitches (v1.1.3) ----");
    add_panel(make_single_switch_program<ActivateMenuGlitch113_Descriptor, ActivateMenuGlitch113>());
    add_panel(make_single_switch_program<CloneItemsBoxCopy2_Descriptor, CloneItemsBoxCopy2>());

    add_divider("---- Glitches (v1.1.2) ----");
    add_panel(make_single_switch_program<ActivateMenuGlitch112_Descriptor, ActivateMenuGlitch112>());

    if (PreloadSettings::instance().DEVELOPER_MODE){
        add_divider("---- Developer Tools ----");
        add_panel(make_single_switch_program<ShinyEncounterTester_Descriptor, ShinyEncounterTester>());
        add_panel(make_single_switch_program<SoundListener_Descriptor, SoundListener>());
    }


    finish_panel_setup();
}



}
}
}
