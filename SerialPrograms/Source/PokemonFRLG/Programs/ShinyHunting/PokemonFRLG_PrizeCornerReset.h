/*  Prize Corner Reset
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_PrizeCornerReset_H
#define PokemonAutomation_PokemonFRLG_PrizeCornerReset_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class PrizeCornerReset_Descriptor : public SingleSwitchProgramDescriptor{
public:
    PrizeCornerReset_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class PrizeCornerReset : public SingleSwitchProgramInstance, private ConfigOption::Listener{
public:
    ~PrizeCornerReset();
    PrizeCornerReset();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext &context) override;

    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override{}

private:
    void obtain_prize(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    IntegerEnumDropdownOption SLOT;
    SimpleIntegerOption<uint32_t> NUM_REDEEM;
    StaticTextOption WARNING;

    BooleanCheckBoxOption TAKE_VIDEO;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

private:
    virtual void on_config_value_changed(void* object) override;
    std::string check_amount_redeemed(uint16_t slot_num, uint32_t redeem_num) const;
};

}
}
}
#endif


