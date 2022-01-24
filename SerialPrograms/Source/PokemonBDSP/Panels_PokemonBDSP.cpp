/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Panels_PokemonBDSP.h"

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
#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy.h"
#include "Programs/Glitches/PokemonBDSP_CloneItemsMenuOverlap.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "BD/SP", listener)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- General ----");
    add_settings<MassRelease_Descriptor, MassRelease>();
    add_program<AutonomousBallThrower_Descriptor, AutonomousBallThrower>();

    add_divider("---- Trading ----");
    add_settings<SelfBoxTrade_Descriptor, SelfBoxTrade>();
    add_settings<SelfTouchTrade_Descriptor, SelfTouchTrade>();

    add_divider("---- Farming ----");
    add_program<MoneyFarmerRoute212_Descriptor, MoneyFarmerRoute212>();
    add_program<MoneyFarmerRoute210_Descriptor, MoneyFarmerRoute210>();
    add_program<DoublesLeveling_Descriptor, DoublesLeveling>();
    add_program<AmitySquarePickUpFarmer_Descriptor, AmitySquarePickUpFarmer>();
    add_program<GiftBerryReset_Descriptor, GiftBerryReset>();

    add_divider("---- Shiny Hunting ----");
    add_program<StarterReset_Descriptor, StarterReset>();
    add_program<LegendaryReset_Descriptor, LegendaryReset>();
    add_program<ShinyHuntOverworld_Descriptor, ShinyHuntOverworld>();
    add_program<ShinyHuntFishing_Descriptor, ShinyHuntFishing>();
    add_program<ShinyHuntShaymin_Descriptor, ShinyHuntShaymin>();

    add_divider("---- Eggs ----");
    add_program<EggFetcher_Descriptor, EggFetcher>();
    add_program<EggHatcher_Descriptor, EggHatcher>();
    add_program<EggAutonomous_Descriptor, EggAutonomous>();

    add_divider("---- Glitches (v1.1.3) ----");
    add_program<ActivateMenuGlitch113_Descriptor, ActivateMenuGlitch113>();
    add_program<CloneItemsBoxCopy2_Descriptor, CloneItemsBoxCopy2>();

    add_divider("---- Glitches (v1.1.2) ----");
    add_program<ActivateMenuGlitch112_Descriptor, ActivateMenuGlitch112>();

    add_divider("---- Glitches (v1.1.1) ----");
    add_program<CloneItemsBoxCopy_Descriptor, CloneItemsBoxCopy>();
    add_program<CloneItemsMenuOverlap_Descriptor, CloneItemsMenuOverlap>();

//    if (GlobalSettings::instance().DEVELOPER_MODE){
//    }


    finish_panel_setup();
}



}
}
}
