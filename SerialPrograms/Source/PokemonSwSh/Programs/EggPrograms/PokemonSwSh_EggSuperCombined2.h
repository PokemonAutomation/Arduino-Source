/*  Egg Super-Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggSuperCombined2_H
#define PokemonAutomation_PokemonSwSh_EggSuperCombined2_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Options/FloatingPointOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepCount.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggSuperCombined2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggSuperCombined2_Descriptor();
};



class EggSuperCombined2 : public SingleSwitchProgramInstance{
public:
    EggSuperCombined2();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    SimpleIntegerOption<uint8_t> BOXES_TO_RELEASE;
    SimpleIntegerOption<uint8_t> BOXES_TO_SKIP;
    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;
    FloatingPointOption FETCHES_PER_BATCH;
    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> SAFETY_TIME;
    TimeExpressionOption<uint16_t> EARLY_HATCH_SAFETY;
    TimeExpressionOption<uint16_t> HATCH_DELAY;
};


}
}
}
#endif
