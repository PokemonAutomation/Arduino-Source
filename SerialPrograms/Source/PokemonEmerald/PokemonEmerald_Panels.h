/*  Pokemon Emerald Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonEmerald_Panels_H
#define PokemonAutomation_PokemonEmerald_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonEmerald{



class PanelListFactory : public PanelListDescriptor{
public:
    PanelListFactory();
    virtual std::vector<PanelEntry> make_panels() const;
};



}
}
}
#endif
