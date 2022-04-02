/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggHatcher_H
#define PokemonAutomation_PokemonSwSh_EggHatcher_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepCount.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggHatcher_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EggHatcher_Descriptor();
};



class EggHatcher : public SingleSwitchProgramInstance{
public:
    EggHatcher(const EggHatcher_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env, CancellableScope& scope) override;


private:
    StartInGripOrGameOption START_IN_GRIP_MENU;

    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> SAFETY_TIME;
    TimeExpressionOption<uint16_t> HATCH_DELAY;
};


}
}
}
#endif
