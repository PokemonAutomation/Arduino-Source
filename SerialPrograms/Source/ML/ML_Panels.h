/*  ML Panels
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_ML_Panels_H
#define PokemonAutomation_ML_Panels_H

#include "CommonFramework/Panels/PanelList.h"

namespace PokemonAutomation{
namespace ML{



class PanelListFactory : public PanelListDescriptor{
public:
    PanelListFactory();
private:
    virtual std::vector<PanelEntry> make_panels() const override;
};



}
}
#endif
