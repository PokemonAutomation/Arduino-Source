/*  Den Roller
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_DenRoller_H
#define PokemonAutomation_PokemonSwSh_DenRoller_H

#include "CommonFramework/Options/StaticTextOption.h"
#include "CommonFramework/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Options/TimeExpressionOption.h"
#include "NintendoSwitch/Options/StartInGripMenuOption.h"
#include "NintendoSwitch/Framework/SingleSwitchProgram.h"
#include "PokemonSwSh/Options/PokemonSwSh_Catchability.h"
#include "PokemonSwSh/Inference/Dens/PokemonSwSh_DenMonReader.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class DenRoller_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    DenRoller_Descriptor();
};



class DenRoller : public SingleSwitchProgramInstance{
public:
    DenRoller(const DenRoller_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;

    void ring_bell(const BotBaseContext& context, int count) const;
    virtual void program(SingleSwitchProgramEnvironment& env) override;

private:
    struct Stats;

    StartInGripOrGameOption START_IN_GRIP_MENU;
    SimpleIntegerOption<uint8_t> SKIPS;
    DenMonSelectOption FILTER;

    CatchabilitySelectorOption CATCHABILITY;
    TimeExpressionOption<uint16_t> VIEW_TIME;

    EventNotificationOption NOTIFICATION_PROGRAM_FINISH;
    EventNotificationsOption NOTIFICATIONS;

    SectionDividerOption m_advanced_options;
    TimeExpressionOption<uint16_t> READ_DELAY;
};




}
}
}
#endif
