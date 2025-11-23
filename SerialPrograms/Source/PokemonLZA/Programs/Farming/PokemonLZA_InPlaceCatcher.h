/*  In-Place Catcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_InPlaceCatcher_H
#define PokemonAutomation_PokemonLZA_InPlaceCatcher_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{

class AlertEyeTracker;


class InPlaceCatcher_Descriptor : public SingleSwitchProgramDescriptor{
public:
    InPlaceCatcher_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class InPlaceCatcher : public SingleSwitchProgramInstance, public ConfigOption::Listener{
public:
    InPlaceCatcher();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void day_night_handler(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void run(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        AlertEyeTracker& attack_tracker,
        ShinySoundHandler& shiny_sound_handler
    );


private:
    SimpleIntegerOption<uint16_t> MAX_BALLS;

    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};






}
}
}
#endif
