/*  Pokemon Home Page Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_PageSwap_H
#define PokemonAutomation_PokemonHome_PageSwap_H

#include "CommonFramework/Options/BooleanCheckBox.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{


class PageSwap : public SingleSwitchProgram{
public:
    PageSwap();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    BooleanCheckBox DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif
