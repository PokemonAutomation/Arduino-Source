/*  Burmy Hunter
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_BurmyFinder_H
#define PokemonAutomation_PokemonLA_BurmyFinder_H

#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_MiscOptions.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BurmyFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    BurmyFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};

class BurmyFinder : public SingleSwitchProgramInstance{
public:
    BurmyFinder();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    struct TreeCounter;

    void run_iteration(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        TreeCounter& tree_counter,
        bool fresh_from_reset
    );
    // Press X to throw pokemon, check whether there is pokemon in the tree. Wait for potential battle
    // after pokemon is thrown.
    // Return true if encountering a burmy in the tree. False otherwise.
    // Also during pokemon throwing and battle, disable shiny sound detection. This is because a shiny burmy
    // will play the shiny sound when it jumps out of the tree. To avoid this shiny sound be categorized as
    // enroute shiny, we disable the sound detection when a burmy may jump out.
    bool check_tree(SingleSwitchProgramEnvironment& env, ProControllerContext& context);
    // Throw pokemon to check a tree.
    // Does not wait for potential battle to start.
    // Unlike `check_tree()`, shiny sound detection is not disabled. 
    void check_tree_no_stop(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // While during battle, press + button to check pokemon details.
    // May throw ProgramFinishedException if a Burmy match is found.
    // Finish battle by mashing A or escape. 
    // Return true if a burmy is in the battle. False otherwise.
    bool handle_battle(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // Set `m_enable_shiny_sound` to false to disable sound detection.
    // Also call `context.wait_for_all_requests()` to make sure controller commands are all executed.
    // See `m_enable_shiny_sound` for more context.
    void disable_shiny_sound(ProControllerContext& context);
    // Re-enable shiny sound detection.
    // Also call `context.wait_for_all_requests()` to make sure controller commands are all executed.
    // See `m_enable_shiny_sound` for more context.
    void enable_shiny_sound(ProControllerContext& context);

    // From any place on the fieldlands, return to Height Camp.
    // It will try to ride on Braviary to escape from attacking pokemon if it can not transport via map.
    // For robustness, the function will detect pokemon battles if for some reason the player character
    // is entering a pokemon battle when this function is called.
    // In the case of an active battle, it calls `handle_battle()` to check potential Burmy and then finish
    // battle. After that, try to go back to Height Camp again.
    void go_to_height_camp(SingleSwitchProgramEnvironment& env, ProControllerContext& context);

    // From camp it will go to trees in sequence instead of heading to camp.
    // It returns the last tree checked so the remain can be checked on single_path
    size_t grouped_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t path, TreeCounter& tree_counter);

    // Check trees individually going from camp based on the last tree checked for each path
    void single_path(SingleSwitchProgramEnvironment& env, ProControllerContext& context, size_t path, size_t last_tree, TreeCounter& tree_counter);


private:
    class RunRoute;

    // Atomic bool to control whether to skip shiny sound for shiny sound detector.
    // Shiny pokemon will play the shiny sound when it jumps out of a tree. To prevent this shiny sound from being
    // treated as enroute shiny (shiny tree pokemon should be regarded as tree shiny), we disable shiny sound
    // detection when throwing pokemon to shake a tree.
    std::atomic<bool> m_enable_shiny_sound{true};

    OCR::LanguageOCROption LANGUAGE;
    StopOnOption STOP_ON;
    ExitBattleMethodOption EXIT_METHOD;

    OverworldShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    EventNotificationOption FOUND_SHINY_OR_ALPHA;
    BattleMatchActionOption MATCH_DETECTED_OPTIONS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
};





}
}
}
#endif
