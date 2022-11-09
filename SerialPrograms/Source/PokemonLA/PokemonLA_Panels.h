/*  Pokemon Legends Arceus Panels
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_Panels_H
#define PokemonAutomation_PokemonLA_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class PanelListFactory : public PanelListDescriptor{
public:
    PanelListFactory();
    virtual std::vector<PanelEntry> make_panels() const;
};



}
}
}
#endif
