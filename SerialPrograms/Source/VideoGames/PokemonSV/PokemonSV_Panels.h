/*  Pokemon Scarlet/Violet Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSV_Panels_H
#define PokemonAutomation_PokemonSV_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{



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
