/*  Egg Combined 2
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggCombined2_H
#define PokemonAutomation_PokemonSwSh_EggCombined2_H

#include "Common/Qt/Options/SimpleIntegerOption.h"
#include "Common/Qt/Options/FloatingPointOption.h"
#include "CommonFramework/Options/StaticTextOption.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepCount.h"
#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggCombined2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggCombined2_Descriptor();
};



class EggCombined2 : public SingleSwitchProgramInstance{
public:
    EggCombined2();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    StartInGripOrGameOption START_IN_GRIP_MENU;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

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
