/*  Egg Super-Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggSuperCombined2_H
#define PokemonAutomation_PokemonSwSh_EggSuperCombined2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/FloatingPoint.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Options/StartInGripMenu.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepCount.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggSuperCombined2_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    EggSuperCombined2_Descriptor();
};



class EggSuperCombined2 : public SingleSwitchProgramInstance{
public:
    EggSuperCombined2(const EggSuperCombined2_Descriptor& descriptor);

    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    StartInGripOrGame START_IN_GRIP_MENU;
    TouchDateInterval TOUCH_DATE_INTERVAL;

    SimpleInteger<uint8_t> BOXES_TO_RELEASE;
    SimpleInteger<uint8_t> BOXES_TO_SKIP;
    SimpleInteger<uint8_t> BOXES_TO_HATCH;
    EggStepCount STEPS_TO_HATCH;
    FloatingPoint FETCHES_PER_BATCH;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> SAFETY_TIME;
    TimeExpression<uint16_t> EARLY_HATCH_SAFETY;
    TimeExpression<uint16_t> HATCH_DELAY;
};


}
}
}
#endif
