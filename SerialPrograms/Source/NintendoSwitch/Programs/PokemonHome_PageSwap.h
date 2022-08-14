/*  Pokemon Home Page Swap
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonHome_PageSwap_H
#define PokemonAutomation_PokemonHome_PageSwap_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    BooleanCheckBoxOption DODGE_SYSTEM_UPDATE_WINDOW;
};


}
}
}
#endif
