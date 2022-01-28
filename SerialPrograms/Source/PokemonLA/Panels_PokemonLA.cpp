/*  Pokemon LA Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/GlobalSettingsPanel.h"
#include "Panels_PokemonLA.h"

#include "PokemonLA_Settings.h"
#include "PokemonLA/Programs/PokemonLA_OverworldWatcher.h"

#include "Programs/PokemonLA_ShinyHunt-LakeTrio.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "PLA", listener)
{
    add_divider("---- Settings ----");
    add_settings<GameSettings_Descriptor, GameSettingsPanel>();
    add_program<OverworldWatcher_Descriptor, OverworldWatcher>();


    if (GlobalSettings::instance().DEVELOPER_MODE){
//        add_divider("---- Shiny Hunting ----");
//        add_program<ShinyHuntLakeTrio_Descriptor, ShinyHuntLakeTrio>();
    }

    finish_panel_setup();
}




}
}
}
