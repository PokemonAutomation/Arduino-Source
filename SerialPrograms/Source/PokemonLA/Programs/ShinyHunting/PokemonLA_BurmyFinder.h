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
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    void check_tree(SingleSwitchProgramEnvironment& env, BotBaseContext& context);

private:
    class Stats;
    class RunRoute;

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
