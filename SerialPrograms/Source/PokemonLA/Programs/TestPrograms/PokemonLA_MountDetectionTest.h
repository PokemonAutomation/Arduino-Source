/*  Mount Detection Test
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    MountDetectorLoggingOption FAILED_ACTION;
};



}
}
}
#endif
