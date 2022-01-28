/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_Panels_H
#define PokemonAutomation_PokemonBDSP_Panels_H

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
