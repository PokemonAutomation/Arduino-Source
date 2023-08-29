/*  Shiny Hunt - Scatterbug
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntScatterbug_H
#define PokemonAutomation_PokemonSV_ShinyHuntScatterbug_H

#include <functional>
//#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class LetsGoHpWatcher;
class DiscontiguousTimeTracker;
class LetsGoEncounterBotTracker;



class ShinyHuntScatterbug_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntScatterbug_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntScatterbug : public SingleSwitchProgramInstance{
public:
    ShinyHuntScatterbug();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    void reset_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    
    void run_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t path_id);

    OCR::LanguageOCROption LANGUAGE;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    FloatingPointOption AUTO_HEAL_PERCENT;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    LetsGoHpWatcher* m_hp_watcher;
    DiscontiguousTimeTracker* m_time_tracker;
    LetsGoEncounterBotTracker* m_encounter_tracker;

    uint64_t m_iterations = 0;

    //  Set to true if we should save on the first available opportunity.
    bool m_pending_save;

    size_t m_consecutive_failures;
};





}
}
}
#endif
