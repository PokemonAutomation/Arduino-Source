/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_OutbreakFinder_H
#define PokemonAutomation_PokemonLA_OutbreakFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonTools/Options/StringSelectTableOption.h"
#include "CommonTools/Options/LanguageOCROption.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OutbreakFinder_Descriptor : public SingleSwitchProgramDescriptor{
public:
    OutbreakFinder_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class OutbreakFinder : public SingleSwitchProgramInstance{
public:
    OutbreakFinder();
    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;


private:
    // Run one iteration of the outbreak finder loop and return any found outbreaks.
    // The iteration includes:
    // 1. Starting at Jubilife Village gate, go to check the map for outbreaks.
    // 2. If found desired outbreaks, stop.
    // 3. If need to check MMOs, save in front of gate, then go to each region with MMO and talk to Mai to
    //    reveal MMO pokemon. Reset if no desired MMO to conserve Aguav Berries. 
    // 4. If found desired MMO pokemon, stop.
    // 5. No desired outbreak in this iteration, go to an arbitrary region and return to village to refresh outbreaks.
    //
    // - desired_hisui_map_events: desired events happening on the travel map of Hisui when leaving Jubilife Village.
    //   It contains desired pokemon outbreak names and MMO outbreak names (e.g. "fieldlands-mmo"). If there are
    //   desired MMO pokemon (including those with star symbols), the MMO outbreaks that may spawn them are also
    //   included in `desired_hisui_map_events`.
    // - desired_outbreaks: desired events happening on the travel map of Hisui when leaving Jubilife Village.
    //   This includes any user selected outbreak pokemon and user explicitly selected MMO events (e.g. "fieldlands-mmo")
    //   in DESIRED_MO_SLUGS.
    //   User selected MMO pokemon (including those with star symbols) do not affect `desired_outbreaks`.
    // - desired_MMO_pokemon: user desired MMO pokemon selected by `DESIRED_MMO_SLUGS`.
    //   User selected MMO pokemon with star symbols, `DESIRED_STAR_MMO_SLUGS` do not affect `desired_MMO_pokemon`. 
    // - desired_star_MMO_pokemon: user desired MMO pokemon with star symbols.
    std::vector<std::string> run_iteration(SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        const std::set<std::string>& desired_hisui_map_events,
        const std::set<std::string>& desired_outbreaks,
        const std::set<std::string>& desired_MMO_pokemon,
        const std::set<std::string>& desired_star_MMO_pokemon,
        bool& fresh_from_reset
    );
    
    //  Read the travel map from Jublilife village to find any desired pokemon or MMO events. 
    //  Return true if program should stop (match found).
    //  desired_events: the desired set of pokemon slugs and MMO events.
    std::set<std::string> read_travel_map_outbreaks(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        const std::set<std::string>& desired_events
    );

    // Go to a random wild region and return to refresh outbreaks.
    // If `inside_map` is true, the function is called when the game is inside the travel map.
    // Otherwise, the function is called when the player character is standing at the program start location.
    void goto_region_and_return(
        SingleSwitchProgramEnvironment& env, ProControllerContext& context,
        bool inside_map,
        bool fresh_from_reset
    );

    static std::set<std::string> to_set(const StringSelectTableOption& option);


private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;
    BooleanCheckBoxOption RESET_GAME_AND_CONTINUE_SEARCHING;

    OCR::LanguageOCROption LANGUAGE;

    StringSelectTableOption DESIRED_MO_SLUGS;
    StringSelectTableOption DESIRED_MMO_SLUGS;
    StringSelectTableOption DESIRED_STAR_MMO_SLUGS;

    BooleanCheckBoxOption DEBUG_MODE;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_MATCHED;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
