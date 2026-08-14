/*  Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonRSE_GiftReset_H
#define PokemonAutomation_PokemonRSE_GiftReset_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonRSE{

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
    void obtain_fossils(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    bool try_open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    uint64_t open_summary(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    enum class Target{
        fossils,
        castform,
        beldum,
        //gen_2_starters, //only if emerald rng is fixed
    };
    EnumDropdownOption<Target> TARGET;

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



