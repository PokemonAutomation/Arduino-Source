/*  Shiny Hunt - Overworld
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_ShinyHuntOverworld_H
#define PokemonAutomation_PokemonFRLG_ShinyHuntOverworld_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"


namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class ShinyHuntOverworld_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntOverworld_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class ShinyHuntOverworld : public SingleSwitchProgramInstance{
public:
    ShinyHuntOverworld();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override {
    }

private:
    bool find_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context) const;

    enum class TriggerMethod {
        HORIZONTAL_NO_BIAS,
        HORIZONTAL_BIAS_LEFT,
        HORIZONTAL_BIAS_RIGHT,
        VERTICAL_NO_BIAS,
        VERTICAL_BIAS_UP,
        VERTICAL_BIAS_DOWN
    };


    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;
    EnumDropdownOption<TriggerMethod> TRIGGER_METHOD;
    MillisecondsOption MOVE_DURATION0;

    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif