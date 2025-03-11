/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenRoller_H
#define PokemonAutomation_PokemonSwSh_DenRoller_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DenRoller_Descriptor : public SingleSwitchProgramDescriptor{
public:
    DenRoller_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class DenRoller : public SingleSwitchProgramInstance{
public:
    DenRoller();

    void ring_bell(ProControllerContext& context, int count) const;
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    StartInGripOrGameOption START_LOCATION;
    SimpleIntegerOption<uint8_t> SKIPS;
    DenMonSelectOption FILTER;

    CatchabilitySelectorOption CATCHABILITY;
    MillisecondsOption VIEW_TIME0;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    MillisecondsOption READ_DELAY0;
};




}
}
}
#endif
