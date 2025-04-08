/*  Egg Autonomous
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonSwSh_EggAutonomous_H
#define PokemonAutomation_PokemonSwSh_EggAutonomous_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Options/NintendoSwitch_StartInGripMenuOption.h"
#include "Pokemon/Options/Pokemon_StatsHuntFilter.h"
#include "PokemonSwSh/Options/PokemonSwSh_DateToucher.h"
// #include "PokemonSwSh/Options/PokemonSwSh_EggStepCount.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSwSh{


class EggAutonomous_Descriptor : public SingleSwitchProgramDescriptor{
public:
    EggAutonomous_Descriptor();
    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class EggAutonomous : public SingleSwitchProgramInstance{
public:
    EggAutonomous();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    // Run one iteration of the egg loop:
    // - Hatch five eggs while fetch five eggs.
    // - Check if pokemon needs to be kept. Keep them if needed.
    // - Put five eggs from storage to party. Save game if needed.
    // Return true if the egg loop should stop.
    bool run_batch(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats
    );

    void save_game(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Call flying taxi to reset player character position to Nursery front door.
    // fly_from_overworld: if true, the game is in the overworld while calling this function. If false, the game is in the menu.
    // Note: the cursor in the menu must already be at Town Map.
    void call_flying_taxi(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        bool fly_from_overworld
    );

    // After detecting egg hatching, call this function to wait entil the end of the hatching.
    // num_hatched_eggs: how many eggs hatched (including the current hatching one)
    void wait_for_egg_hatched(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats,
        size_t num_hatched_eggs
    );

    // Call this function when standing in front of the lady to fetch one egg.
    // Return updated `num_eggs_retrieved` to reflect change in fetched eggs.
    size_t talk_to_lady_to_fetch_egg(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats,
        size_t num_eggs_retrieved
    );

    // After all five eggs hatched and another five eggs deposit into the first column of the box,
    // call this function to:
    // - Go to pokemon storage.
    // - Check the hatched pokemon, keep shiny pokemon and those that match the stats requirements. Release the rest.
    // - Retrieve the stored egg column to the party.
    // - Call flying taxi to reset player location if needed
    // Return true if the program should stop
    bool process_hatched_pokemon(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats,
        bool need_taxi
    );

    // Used to wait until Y-Comm icon shows up.
    // Throw error if it does not find it after 10 sec.
    void mash_B_until_y_comm_icon(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        const std::string& error_msg
    );

    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;
    // EggStepCountOption STEPS_TO_HATCH;
    
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    SimpleIntegerOption<uint8_t> MAX_KEEPERS;
    SimpleIntegerOption<uint8_t> LOOPS_PER_FETCH;
    IntegerEnumDropdownOption NUM_EGGS_IN_COLUMN;

    enum class AutoSave{
        NoAutoSave,
        AfterStartAndKeep,
        EveryBatch,
    };
    EnumDropdownOption<AutoSave> AUTO_SAVING;
    
    Pokemon::StatsHuntIvJudgeFilterTable FILTERS0;

    BooleanCheckBoxOption DEBUG_PROCESSING_HATCHED;
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY_KEEP;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationOption m_notification_noop;
    EventNotificationsOption NOTIFICATIONS;

    // How many pokemon have been kept so far. These pokemon are shiny or met certain stats requirement.
    size_t m_num_pokemon_kept = 0;

    // How many eggs have been placed behind a game save.
    // This is used so that if we recover from an error, we know how many eggs are in storage.
    size_t m_num_eggs_in_storage_when_game_saved = 0;
    // How many eggs are already deposited to storage so far.
    size_t m_num_eggs_retrieved = 0;

    // Is player's location at the bike loop start
    bool m_player_at_loop_start = false;
};


}
}
}
#endif
