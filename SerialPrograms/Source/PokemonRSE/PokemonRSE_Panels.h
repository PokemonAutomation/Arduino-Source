/*  Pokemon RSE Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_Panels_H
#define PokemonAutomation_PokemonRSE_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{



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
