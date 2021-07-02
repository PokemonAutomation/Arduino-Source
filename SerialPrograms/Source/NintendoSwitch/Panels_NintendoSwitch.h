/*  Nintendo Switch Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitchPanels_H
#define PokemonAutomation_NintendoSwitchPanels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class Panels : public PanelList{
public:
    Panels(QTabWidget& parent, PanelListener& listener);
};



}
}
#endif
