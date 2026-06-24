/*  Held Item Farmer - Safari Zone
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonFRLG_HeldItemFarmer_SafariZone_H
#define PokemonAutomation_PokemonFRLG_HeldItemFarmer_SafariZone_H

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

class HeldItemFarmerSafariZone_Descriptor : public SingleSwitchProgramDescriptor{
public:
    HeldItemFarmerSafariZone_Descriptor();
    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class HeldItemFarmerSafariZone : public SingleSwitchProgramInstance {
public:
    HeldItemFarmerSafariZone();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;
    virtual void start_program_border_check(
        VideoStream& stream,
        FeedbackType feedback_type
    ) override {
    }

private:
    enum class ItemToFarm{
        LUCKY_EGG,
        DRAGON_FANG
    };


    // After exiting the safari zone building navigate to grass with Chansey.
    // Currently only supports running. Should add Surf option...
    bool navigate_to_chansey(ConsoleHandle& console, ProControllerContext& context);
    // After exiting the safari zone building navigate to water with Dragonair.
    // Currently only supports running.
    void navigate_to_dragonair(ConsoleHandle& console, ProControllerContext& context);
    // Swap first and second pokemon. 
    // First pokemon used to avoid encounters on the route to Chansey. Second pokemon used to improve encounter rates.
    void swap_lead_pokemon(ConsoleHandle& console, ProControllerContext& context);
    // Reads wild encounter name and returns true if Chansey, uses a set list of possible names in the expected area.
    bool is_chansey(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Reads wild encounter name and returns true if Dragonair, uses a set list of possible names in the expected area.
    bool is_dragonair(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handles the grass encounter logic. Attempts to spin in place. Resets position to the top right corner of grass.
    // returns -1 if no encounter is triggered, 0 if a non-shiny is encountered, and 1 if a shiny is encountered
    int find_encounter_grass(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Handles the catch logic. Should be updated to throw bait for better catch rates.
    // Returns true if catch successful. Returns in the overworld.
    bool attempt_catch(SingleSwitchProgramEnvironment& env, ProControllerContext& context, int& balls_left);
    // Opens the party menu from a given overworld location (safari zone, or the main safari zone building)
    // Checks the last four party slots for held items. Returns true if item detected.
    bool check_for_held_item(ConsoleHandle& console, ProControllerContext& context, bool returned_to_building);
    // Handles the main loop once we are in the grass ready to search for a Chansey. 
    // Returns true if a stop condition is met (lucky egg or shiny found).
    // Returns false if we need to soft reset (out of safari balls, out of steps, caught a full party of Chansey).
    bool run_safari_zone(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    EnumDropdownOption<ItemToFarm> ITEM_TO_FARM;
    OCR::LanguageOCROption LANGUAGE;

    DeferredStopButtonOption STOP_AFTER_CURRENT;
    BooleanCheckBoxOption TAKE_VIDEO;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_HELD_ITEM;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

};

}
}
}
     



#endif
