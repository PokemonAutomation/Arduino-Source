/*  ML Run YOLO Program
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *  Test YOLO detection model on Switch streams.
 */

#ifndef PokemonAutomation_ML_RunYOLO_H
#define PokemonAutomation_ML_RunYOLO_H

#include <memory>
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "ML/Models/ML_YOLOv5Model.h"

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
    std::unique_ptr<YOLOv5Session> m_yolo_session;
};


}
}
#endif



