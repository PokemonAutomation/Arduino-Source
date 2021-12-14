/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Panels_PokemonBDSP.h"

#include "PokemonBDSP_Settings.h"

#include "Programs/General/PokemonBDSP_MassRelease.h"

#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute210.h"
#include "Programs/Farming/PokemonBDSP_MoneyFarmerRoute212.h"
#include "Programs/Farming/PokemonBDSP_DoublesLeveling.h"

#include "Programs/ShinyHunting/PokemonBDSP_StarterReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_LegendaryReset.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Overworld.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-Fishing.h"
#include "Programs/ShinyHunting/PokemonBDSP_ShinyHunt-ShayminRunaway.h"

#include "Programs/Eggs/PokemonBDSP_EggFetcher.h"
#include "Programs/Eggs/PokemonBDSP_EggHatcher.h"

#include "Programs/Glitches/PokemonBDSP_CloneItemsMenuOverlap.h"
#include "Programs/Glitches/PokemonBDSP_CloneItemsBoxCopy.h"

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

    add_divider("---- Farming ----");
    add_program<MoneyFarmerRoute212_Descriptor, MoneyFarmerRoute212>();
    add_program<MoneyFarmerRoute210_Descriptor, MoneyFarmerRoute210>();
    add_program<DoublesLeveling_Descriptor, DoublesLeveling>();

    add_divider("---- Shiny Hunting ----");
    add_program<StarterReset_Descriptor, StarterReset>();
    add_program<LegendaryReset_Descriptor, LegendaryReset>();
    add_program<ShinyHuntOverworld_Descriptor, ShinyHuntOverworld>();
    add_program<ShinyHuntFishing_Descriptor, ShinyHuntFishing>();
    add_program<ShinyHuntShayminRunaway_Descriptor, ShinyHuntShayminRunaway>();

    add_divider("---- Eggs ----");
    add_program<EggFetcher_Descriptor, EggFetcher>();
    add_program<EggHatcher_Descriptor, EggHatcher>();

//    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_divider("---- Glitches (v1.1.1) ----");
        add_program<CloneItemsMenuOverlap_Descriptor, CloneItemsMenuOverlap>();
        add_program<CloneItemsBoxCopy_Descriptor, CloneItemsBoxCopy>();
//    }


    finish_panel_setup();
}



}
}
}
