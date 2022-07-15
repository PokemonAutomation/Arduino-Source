/*  Outbreak Finder
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_OutbreakFinder_H
#define PokemonAutomation_PokemonLA_OutbreakFinder_H

#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/OCR/OCR_LanguageOptionOCR.h"
#include "NintendoSwitch/Options/GoHomeWhenDoneOption.h"
#include "NintendoSwitch/Framework/NintendoSwitch_SingleSwitchProgram.h"
#include "Pokemon/Options/Pokemon_NameListOption.h"
#include "PokemonLA/PokemonLA_TravelLocations.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{


class OutbreakFinder_Descriptor : public RunnableSwitchProgramDescriptor{
public:
    OutbreakFinder_Descriptor();
};


class OutbreakFinder : public SingleSwitchProgramInstance{
public:
    OutbreakFinder(const OutbreakFinder_Descriptor& descriptor);

    virtual std::unique_ptr<StatsTracker> make_stats() const override;
    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;


private:
    std::set<std::string> read_outbreaks(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        MapRegion& region, const std::set<std::string>& desired
    );

    void goto_region_and_return(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        MapRegion region
    );

    std::set<std::string> read_MMOs(
        SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        const std::string& mmo_name,
        const std::set<std::string>& desired_MMOs,
        const std::set<std::string>& desired_star_MMOs
    );

    bool run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context,
        const std::set<std::string>& desired_hisui_map_events,
        const std::set<std::string>& desired_outbreaks,
        const std::set<std::string>& desired_MMOs,
        const std::set<std::string>& desired_star_MMOs);

private:
    class Stats;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCR LANGUAGE;

    Pokemon::PokemonNameList DESIRED_MO_SLUGS;
    Pokemon::PokemonNameList DESIRED_MMO_SLUGS;
    Pokemon::PokemonNameList DESIRED_STAR_MMO_SLUGS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_MATCHED;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
