/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggHatcher_H
#define PokemonAutomation_PokemonSwSh_EggHatcher_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_EggStepOption.h"
//#include "PokemonSwSh_EggHelpers.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggHatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggHatcher_Descriptor();
};



class EggHatcher : public SingleSwitchProgramInstance{
public:
    EggHatcher();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;

    SectionDividerOption m_advanced_options;
    MillisecondsOption SAFETY_TIME1;
    MillisecondsOption HATCH_DELAY0;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
