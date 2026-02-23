/*  Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_GiftReset_H
#define PokemonAutomation_PokemonFRLG_GiftReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class GiftReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    GiftReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class GiftReset : public SingleSwitchProgramInstance{
public:
    GiftReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    void obtain_pokemon(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void obtain_lapras(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    void open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    enum class Target{
        starters,
        hitmon,
        eevee,
        lapras,
        fossils,
    };
    EnumDropdownOption<Target> TARGET;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;
};

}
}
}
#endif



