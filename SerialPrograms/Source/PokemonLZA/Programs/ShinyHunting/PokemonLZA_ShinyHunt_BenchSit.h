/*  Shiny Hunt - Bench Sit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_BenchSit_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_BenchSit_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_BenchSit_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_BenchSit_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_BenchSit : public SingleSwitchProgramInstance{
public:
    ShinyHunt_BenchSit();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    IntegerEnumDropdownOption WALK_DIRECTION;
    MillisecondsOption WALK_FORWARD_DURATION;
    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
