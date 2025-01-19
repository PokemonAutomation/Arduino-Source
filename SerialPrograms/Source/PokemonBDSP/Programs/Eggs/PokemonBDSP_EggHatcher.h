/*  Egg Hatcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonBDSP_EggHatcher_H
#define PokemonAutomation_PokemonBDSP_EggHatcher_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonBDSP/Options/PokemonBDSP_EggStepOption.h"
//#include "PokemonBDSP/Options/PokemonBDSP_ShortcutDirection.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonBDSP{


class EggHatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggHatcher_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class EggHatcher : public SingleSwitchProgramInstance{
public:
    EggHatcher();

    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;

private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

//    ShortcutDirection SHORTCUT;
    SimpleIntegerOption<uint8_t> BOXES_TO_HATCH;
    EggStepCountOption STEPS_TO_HATCH;

    BooleanCheckBoxOption SAVE_AND_RESET;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> SAFETY_TIME0;
    TimeExpressionOption<uint16_t> HATCH_DELAY;
};




}
}
}
#endif
