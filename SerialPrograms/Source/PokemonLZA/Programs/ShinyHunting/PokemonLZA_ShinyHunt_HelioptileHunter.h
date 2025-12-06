/*  Shiny Hunt - Helioptile Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_HelioptileHunter_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_HelioptileHunter_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_HelioptileHunter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_HelioptileHunter_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_HelioptileHunter : public SingleSwitchProgramInstance{
public:
    ShinyHunt_HelioptileHunter();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    SimpleIntegerOption<uint32_t> END_AFTER_CYCLE;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
