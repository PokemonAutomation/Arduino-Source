/*  Shiny Hunt - Bench
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_Bench_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_Bench_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_Bench_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_Bench_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_Bench : public SingleSwitchProgramInstance{
public:
    ShinyHunt_Bench();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    ShinyDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
