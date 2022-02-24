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
#include "Programs/PokemonLA_NuggetFarmerHighlands.h"

#include "Programs/PokemonLA_SelfBoxTrade.h"
#include "Programs/PokemonLA_SelfTouchTrade.h"

//#include "Programs/PokemonLA_ShinyHunt-LakeTrio.h"
#include "PokemonLA/Programs/PokemonLA_OverworldWatcher.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "PLA", listener)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();

    add_divider("---- General ----");
    add_program<BraviaryHeightGlitch_Descriptor, BraviaryHeightGlitch>();
    add_program<DistortionWaiter_Descriptor, DistortionWaiter>();
    add_program<OutbreakFinder_Descriptor, OutbreakFinder>();

    if (GlobalSettings::instance().DEVELOPER_MODE){
        add_program<NuggetFarmerHighlands_Descriptor, MoneyFarmerHighlands>();
    }

    add_divider("---- Trading ----");
    add_program<SelfBoxTrade_Descriptor, SelfBoxTrade>();
    add_program<SelfTouchTrade_Descriptor, SelfTouchTrade>();


    if (GlobalSettings::instance().DEVELOPER_MODE){
//        add_divider("---- Shiny Hunting ----");
//        add_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>();
        add_divider("---- Developer Tools ----");
        add_program<OverworldWatcher_Descriptor, OverworldWatcher>();
    }

    finish_panel_setup();
}




}
}
}
