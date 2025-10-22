/*  Beldum Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_BeldumHunter_H
#define PokemonAutomation_PokemonLZA_BeldumHunter_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{


class BeldumHunter_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BeldumHunter_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class BeldumHunter : public SingleSwitchProgramInstance{
public:
    BeldumHunter();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    bool run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    BooleanCheckBoxOption TAKE_VIDEO;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};


}
}
}
#endif
