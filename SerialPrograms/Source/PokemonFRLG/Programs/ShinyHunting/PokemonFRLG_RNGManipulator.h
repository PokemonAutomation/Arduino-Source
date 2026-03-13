/*  Gift Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_RNGManipulator_H
#define PokemonAutomation_PokemonFRLG_RNGManipulator_H

#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class RNGManipulator_Descriptor : public SingleSwitchProgramDescriptor{
public:
    RNGManipulator_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class RNGManipulator : public SingleSwitchProgramInstance{
public:
    RNGManipulator();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    enum class ResetType{
        hard,
        soft,
    };
    enum class Target{
        starters,
        magikarp,
        // hitmon,
        eevee,
        // lapras,
        // fossils,
        snorlax,
        sweetscent,
        // fishing,
        // static,
        // roaming
    };

    EnumDropdownOption<Target> TARGET;

    SimpleIntegerOption<uint64_t> NUM_RESETS;

    EnumDropdownOption<ResetType> RESET_TYPE;

    SimpleIntegerOption<uint64_t> SEED_DELAY;
    SimpleIntegerOption<uint64_t> LOAD_ADVANCES;
    SimpleIntegerOption<uint64_t> DOUBLE_ADVANCES;

    BooleanCheckBoxOption TAKE_PICTURES;

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



