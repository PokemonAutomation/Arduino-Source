/*  Pokemon Sword/Shield Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwordShieldPanels_H
#define PokemonAutomation_PokemonSwordShieldPanels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class Panels : public PanelList{
public:
    Panels(QTabWidget& parent, PanelListener& listener);
};



}
}
}
#endif
