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
#include "PokemonLA/Inference/PokemonLA_SelectedRegionDetector.h"

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
    virtual void program(SingleSwitchProgramEnvironment& env) override;


private:
    bool read_outbreaks(SingleSwitchProgramEnvironment& env, MapRegion& region, const std::set<std::string>& desired);
    bool goto_region_and_return(SingleSwitchProgramEnvironment& env, MapRegion region);

private:
    class Stats;

    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    OCR::LanguageOCR LANGUAGE;

    Pokemon::PokemonNameList DESIRED_SLUGS;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationOption NOTIFICATION_MATCHED;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
