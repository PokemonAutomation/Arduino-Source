/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PreventSleep_H
#define PokemonAutomation_NintendoSwitch_PreventSleep_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class PreventSleep_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PreventSleep_Descriptor();
};


class PreventSleep : public SingleSwitchProgramInstance{
public:
    PreventSleep();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


};



}
}
#endif



