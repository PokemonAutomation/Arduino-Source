/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "CommonFramework/Windows/MainWindow.h"
#include "Panels_PokemonBDSP.h"



namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


Panels::Panels(QTabWidget& parent, PanelListener& listener)
    : PanelList(parent, "BD/SP", listener)
{
    PersistentSettings& settings = PERSISTENT_SETTINGS();


    finish_panel_setup();
}



}
}
}
