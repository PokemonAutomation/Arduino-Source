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


class PreventSleep : public SingleSwitchProgram{
public:
    PreventSleep();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;


};



}
}
#endif



