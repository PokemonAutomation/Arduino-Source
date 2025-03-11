/*  Egg Super-Combined 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggSuperCombined2_H
#define PokemonAutomation_PokemonSwSh_EggSuperCombined2_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepOption.h"

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

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

    SimpleIntegerOption<uint8_t> BOXES_TO_RELEASE;
    SimpleIntegerOption<uint8_t> BOXES_TO_SKIP;
    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;
    FloatingPointOption FETCHES_PER_BATCH;

    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption SAFETY_TIME0;
    MillisecondsOption EARLY_HATCH_SAFETY0;
    MillisecondsOption HATCH_DELAY0;
};


}
}
}
#endif
