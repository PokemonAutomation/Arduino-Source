/*  ML Run YOLO Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Test YOLO detection model on Switch streams.
 */

#ifndef PokemonAutomation_ML_RunYOLO_H
#define PokemonAutomation_ML_RunYOLO_H

#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace ML{


class RunYOLO_Descriptor : public NintendoSwitch::SingleSwitchProgramDescriptor{
public:
    RunYOLO_Descriptor();
};


class RunYOLO : public NintendoSwitch::SingleSwitchProgramInstance{
public:
    RunYOLO();

    virtual void program(NintendoSwitch::SingleSwitchProgramEnvironment& env, NintendoSwitch::ProControllerContext& context) override;

private:
};


}
}
#endif



