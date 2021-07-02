/*  Prevent Sleep
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_PreventSleep_H
#define PokemonAutomation_NintendoSwitch_PreventSleep_H

#include "NintendoSwitch/Framework/SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class PreventSleep_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    PreventSleep_Descriptor();
};


class PreventSleep : public SingleSwitchProgramInstance{
public:
    PreventSleep(const PreventSleep_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;


};



}
}
#endif



