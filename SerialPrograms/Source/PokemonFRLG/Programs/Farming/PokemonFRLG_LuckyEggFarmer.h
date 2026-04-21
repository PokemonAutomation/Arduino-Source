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

    // After exiting the safari zone building navigate to grass with Chansey.
    // Currently only supports running. Should add Surf option...
    bool navigate_to_chansey(ConsoleHandle& console, ProControllerContext& context);
    // Swap first and second pokemon. 
    // First pokemon used to avoid encounters on the route to Chansey. Second pokemon used to improve encounter rates.
    void swap_lead_pokemon(ConsoleHandle& console, ProControllerContext& context);
    // Reads wild encounter name and returns true if Chansey
    bool is_chansey(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handles the encounter logic. Attempts to spin in place. Resets position to the top right corner of grass.
    // Returns true if transition to battle detected.
    bool find_encounter(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handles the catch logic. Should be updated to throw bait for better catch rates.
    // Returns true if catch successful. Returns in the overworld.
    bool attempt_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int& balls_left);
    // Opens the party menu from a given overworld location (safari zone, or the main safari zone building)
    // Checks the last four party slots for held items. Returns true if item detected.
    bool check_for_lucky_egg(ConsoleHandle& console, ProControllerContext& context, bool returned_to_building);
    // Handles the main loop once we are in the grass ready to search for a Chansey. 
    // Returns true if a stop condition is met (lucky egg or shiny found).
    // Returns false if we need to soft reset (out of safari balls, out of steps, caught a full party of Chansey).
    bool run_safari_zone(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    OCR::LanguageOCROption LANGUAGE;

    DeferredStopButtonOption STOP_AFTER_CURRENT;
    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_LUCKY_EGG;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

};

}
}
}
     



#endif
