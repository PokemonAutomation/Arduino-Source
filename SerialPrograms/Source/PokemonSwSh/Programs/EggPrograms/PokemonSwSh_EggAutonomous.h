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

enum class EggAutoPhase{
    BIKE_LOOP,
    HATCHING,
    FLY_RESET,
    FETCH_EGG,

};

struct EggFetchResult{
    size_t num_eggs_retrieved;
    bool hatch_detected;
};


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


    // return true if egg hatching detected during the bike loop
    bool run_bike_loop(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );

    void exceed_bike_loop_limit(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        size_t max_bike_loop_count
    );

    // Return updated `num_eggs_hatched` to reflect change in hatched eggs.
    size_t hatch_routine(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats,
        size_t num_eggs_hatched
    );

    // Call flying taxi to reset player character position to Nursery front door.
    // fly_from_overworld: if true, the game is in the overworld while calling this function. If false, the game is in the menu.
    // Note: the cursor in the menu must already be at Town Map.
    // return true if egg hatching detected while trying to open Rotom phone menu
    bool call_flying_taxi(
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
    // return EggFetchResult, which is a struct of the following:
    //  - num_eggs_retrieved: updated `num_eggs_retrieved` that reflects the change in fetched eggs.
    //  - hatch_detected: boolean that is true if hatch was detected.
    EggFetchResult talk_to_lady_to_fetch_egg(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        EggAutonomous_Descriptor::Stats& stats,
        size_t num_eggs_retrieved,
        size_t num_eggs_to_fetch
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



    StartInGripOrGameOption START_LOCATION;
    TouchDateIntervalOption TOUCH_DATE_INTERVAL;
    // EggStepCountOption STEPS_TO_HATCH;
    
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCROption LANGUAGE;

    SimpleIntegerOption<uint8_t> MAX_KEEPERS;
    SimpleIntegerOption<uint8_t> LOOPS_PER_FETCH;
    IntegerEnumDropdownOption NUM_EGGS_IN_COLUMN;
    IntegerEnumDropdownOption NUM_EGGS_IN_PARTY;

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

    // This is used so that if we recover from an error, we know how many eggs are in the party
    size_t m_num_eggs_in_party_when_game_saved = 0;

    // How many eggs are already deposited to storage so far.
    size_t m_num_eggs_retrieved = 0;

    // number of eggs in party at the start of run_batch()
    size_t m_num_eggs_in_party_at_batch_start = 0;

    // Is player's location at the bike loop start
    bool m_player_at_loop_start = false;
};


}
}
}
#endif
