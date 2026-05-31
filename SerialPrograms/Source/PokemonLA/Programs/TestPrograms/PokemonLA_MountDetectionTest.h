/*  Mount Detection Test
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_MountDetectionTest_H
#define PokemonAutomation_PokemonLA_MountDetectionTest_H

#include "PokemonLA/Inference/PokemonLA_MountDetector.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



class MountDetectionTest_Descriptor : public SingleSwitchProgramDescriptor{
public:
    MountDetectionTest_Descriptor();
};


class MountDetectionTest : public SingleSwitchProgramInstance{
public:
    MountDetectionTest();

    virtual void start_program_controller_check(ControllerSession& session) override{}
    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;

private:
    MountDetectorLoggingOption FAILED_ACTION;
};



}
}
}
#endif
