/*  Joycon Test Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_NintendoSwitch_JoyconProgram_H
#define PokemonAutomation_NintendoSwitch_JoyconProgram_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


class JoyconProgram_Descriptor : public SingleSwitchProgramDescriptor{
public:
    JoyconProgram_Descriptor();
};



class JoyconProgram : public SingleSwitchProgramInstance{
public:
    JoyconProgram();
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

};




}
}
#endif



