/*  Pokemon BD/SP Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_Panels_H
#define PokemonAutomation_PokemonBDSP_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{



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
