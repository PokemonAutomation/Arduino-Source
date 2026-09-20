/*  Pokemon Legends ZA
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_Panels_H
#define PokemonAutomation_PokemonLZA_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



class PanelListFactory : public PanelListDescriptor{
public:
    PanelListFactory();
private:
    virtual std::vector<PanelEntry> make_panels() const override;
};



}
}
}
#endif
