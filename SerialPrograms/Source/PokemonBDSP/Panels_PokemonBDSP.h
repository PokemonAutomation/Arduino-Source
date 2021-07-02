/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSPPanels_H
#define PokemonAutomation_PokemonBDSPPanels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class Panels : public PanelList{
public:
    Panels(QTabWidget& parent, PanelListener& listener);
};



}
}
}
#endif
