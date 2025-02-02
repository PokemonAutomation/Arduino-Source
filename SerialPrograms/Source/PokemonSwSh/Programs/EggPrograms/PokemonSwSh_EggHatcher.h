/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggHatcher_H
#define PokemonAutomation_PokemonSwSh_EggHatcher_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
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

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;


private:
    StartInGripOrGameOption START_LOCATION;

    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> SAFETY_TIME0;
    TimeExpressionOption<uint16_t> HATCH_DELAY;

    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
