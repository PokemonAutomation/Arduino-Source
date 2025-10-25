/*  Shiny Hunt - Overworld Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_OverworldReset_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_OverworldReset_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_OverworldReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_OverworldReset_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_OverworldReset : public SingleSwitchProgramInstance{
public:
    ShinyHunt_OverworldReset();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    MillisecondsOption RESET_DELAY;
    BooleanCheckBoxOption ROTATE_CAMERA;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
