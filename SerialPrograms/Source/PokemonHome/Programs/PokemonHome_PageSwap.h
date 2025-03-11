/*  Pokemon Home Page Swap
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonHome_PageSwap_H
#define PokemonAutomation_PokemonHome_PageSwap_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonHome{

class PageSwap_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PageSwap_Descriptor();
};


class PageSwap : public SingleSwitchProgramInstance{
public:
    PageSwap();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    BooleanCheckBoxOption DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif
