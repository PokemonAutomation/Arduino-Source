/*  Egg Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggCombined2_H
#define PokemonAutomation_PokemonSwSh_EggCombined2_H

#include "CommonFramework/Options/SectionDivider.h"
#include "CommonFramework/Options/SimpleInteger.h"
#include "CommonFramework/Options/FloatingPoint.h"
#include "NintendoSwitch/Options/TimeExpression.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/EggStepCount.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{

class EggCombined2 : public SingleSwitchProgram{
public:
    EggCombined2();

    virtual void program(SingleSwitchProgramEnvironment& env) const override;

private:
    SimpleInteger<uint8_t> BOXES_TO_HATCH;
    EggStepCount STEPS_TO_HATCH;
    FloatingPoint FETCHES_PER_BATCH;
    SectionDivider m_advanced_options;
    TimeExpression<uint16_t> SAFETY_TIME;
    TimeExpression<uint16_t> EARLY_HATCH_SAFETY;
    TimeExpression<uint16_t> HATCH_DELAY;
    TimeExpression<uint32_t> TOUCH_DATE_INTERVAL;
};


}
}
}
#endif
