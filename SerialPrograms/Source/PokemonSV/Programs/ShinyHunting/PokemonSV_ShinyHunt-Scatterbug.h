/*  Shiny Hunt - Scatterbug
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntScatterbug_H
#define PokemonAutomation_PokemonSV_ShinyHuntScatterbug_H

#include <functional>
#include "Common/Cpp/Options/FloatingPointOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV/Options/PokemonSV_SandwichMakerOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class DiscontiguousTimeTracker;
class LetsGoEncounterBotTracker;
class EncounterWatcher;



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
    void run_one_sandwich_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    
    void reset_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context);
    
    void run_lets_go_iteration(SingleSwitchProgramEnvironment& env, BotBaseContext& context, size_t path_id);

    void handle_battles_and_back_to_pokecenter(SingleSwitchProgramEnvironment& env, BotBaseContext& context, 
        std::function<void(SingleSwitchProgramEnvironment& env, BotBaseContext& context)>&& action);

    OCR::LanguageOCROption LANGUAGE;

    SandwichMakerOption SANDWICH_OPTIONS;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    FloatingPointOption AUTO_HEAL_PERCENT;

    // Debug options
    BooleanCheckBoxOption SAVE_DEBUG_VIDEO;
    BooleanCheckBoxOption DEBUG_WARP_TO_POKECENTER;
    BooleanCheckBoxOption SKIP_SANDWICH;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    LetsGoEncounterBotTracker* m_encounter_tracker;

    //  Set to true if we should save on the first available opportunity.
    bool m_pending_save;
};





}
}
}
#endif
