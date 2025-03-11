/*  Egg Combined 2
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggCombined2_H
#define PokemonAutomation_PokemonSwSh_EggCombined2_H

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


class EggCombined2_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggCombined2_Descriptor();
};



class EggCombined2 : public SingleSwitchProgramInstance{
public:
    EggCombined2();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;

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
