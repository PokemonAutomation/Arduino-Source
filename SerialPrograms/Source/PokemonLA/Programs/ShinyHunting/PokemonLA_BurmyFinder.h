/*  Burmy Hunter
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_BurmyFinder_H
#define PokemonAutomation_PokemonLA_BurmyFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class BurmyFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    BurmyFinder_Descriptor();
};

class BurmyFinder : public SingleSwitchProgramInstance{
public:
    BurmyFinder(const BurmyFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    struct TreeCounter;

    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, TreeCounter& tree_counter);
    // Press X to throw pokemon, check whether there is pokemon in the tree. Wait for potential battle
    // after pokemon is thrown.
    // Return true if encountering a burmy in the tree. False otherwise.
    // Also during pokemon throwing and battle, disable shiny sound detection. This is because a shiny burmy
    // will play the shiny sound when it jumps out of the tree. To avoid this shiny sound be categorized as
    // enroute shiny, we disable the sound detection when a burmy may jump out.
    bool check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    // Throw pokemon to check a tree.
    // Does not wait for potential battle to start.
    // Unlike `check_tree()`, shiny sound detection is not disabled. 
    void check_tree_no_stop(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    // While during battle, press + button to check pokemon details.
    // May throw ProgramFinishedException if a Burmy match is found.
    // Finish battle by mashing A or escape. 
    // Return true if a burmy is in the battle. False otherwise.
    bool handle_battle(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

    void disable_shiny_sound(BotBaseContext& context);
    void enable_shiny_sound(BotBaseContext& context);

private:
    class Stats;
    class RunRoute;

    std::atomic<bool> m_enable_shiny_sound{true};

    OCR::LanguageOCR LANGUAGE;
    EnumDropdownOption STOP_ON;
    EnumDropdownOption EXIT_METHOD;
    ShinyDetectedActionOption SHINY_DETECTED_ENROUTE;
    ShinyDetectedActionOption MATCH_DETECTED_OPTIONS;
    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
