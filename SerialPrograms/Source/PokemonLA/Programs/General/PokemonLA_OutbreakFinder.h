/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
    virtual void program(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context) override;


private:
    // One iteration in the main program loop
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
    bool run_iteration(SingleSwitchProgramEnvironment& env, SwitchControllerContext& context,
        const std::set<std::string>& desired_hisui_map_events,
        const std::set<std::string>& desired_outbreaks,
        const std::set<std::string>& desired_MMO_pokemon,
        const std::set<std::string>& desired_star_MMO_pokemon);
    
    //  Read the travel map from Jublilife village to find any desired pokemon or MMO events. 
    //  Return true if program should stop (match found).
    //  desired_events: the desired set of pokemon slugs and MMO events.
    std::set<std::string> read_travel_map_outbreaks(
        SingleSwitchProgramEnvironment& env, SwitchControllerContext& context,
        const std::set<std::string>& desired_events
    );

    // Enter a map with MMO and read names of the pokemon appearing in the MMO.
    // - mmo_name: MMO event slug, e.g. "fieldlands-mmo"
    std::set<std::string> enter_region_and_read_MMO(
        SingleSwitchProgramEnvironment& env, SwitchControllerContext& context,
        const std::string& mmo_name,
        const std::set<std::string>& desired_MMOs,
        const std::set<std::string>& desired_star_MMOs
    );

    // Go to a random wild region and return to refresh outbreaks.
    // If `inside_map` is true, the function is called when the game is inside the travel map.
    // Otherwise, the function is called when the player character is standing at the program start location.
    void goto_region_and_return(
        SingleSwitchProgramEnvironment& env, SwitchControllerContext& context,
        bool inside_map
    );

    static std::set<std::string> to_set(const StringSelectTableOption& option);


private:
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

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
