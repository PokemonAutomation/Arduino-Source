/*  Lucky Egg Farmer
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_LuckyEggFarmer_H
#define PokemonAutomation_PokemonFRLG_LuckyEggFarmer_H

#include "Common/Cpp/Options/ButtonOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_ConsoleHandle.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonFRLG{

class LuckyEggFarmer_Descriptor : public SingleSwitchProgramDescriptor{
public:
    LuckyEggFarmer_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class LuckyEggFarmer : public SingleSwitchProgramInstance {
public:
    LuckyEggFarmer();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override {
    }

private:

    bool navigate_to_chansey(ConsoleHandle& console, ProControllerContext& context);
	void swap_lead_pokemon(ConsoleHandle& console, ProControllerContext& context);
	bool is_chansey(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
	bool find_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
	bool attempt_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int& balls_left);
    bool check_for_lucky_egg(ConsoleHandle& console, ProControllerContext& context);

    OCR::LanguageOCROption LANGUAGE;

    DeferredStopButtonOption STOP_AFTER_CURRENT;
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
