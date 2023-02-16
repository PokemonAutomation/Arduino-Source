/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H
#define PokemonAutomation_PokemonSV_ShinyHuntAreaZeroPlatform_H

#include <functional>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Options/LanguageOCROption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV/Options/PokemonSV_EncounterBotCommon.h"
#include "PokemonSV_AreaZeroPlatform.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class LetsGoEncounterBotTracker;



class ShinyHuntAreaZeroPlatform_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAreaZeroPlatform_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};



class ShinyHuntAreaZeroPlatform : public SingleSwitchProgramInstance{
public:
    ShinyHuntAreaZeroPlatform();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    enum class State{
        TRAVERSAL,
        INSIDE_GATE_AND_RETURN,
        LEAVE_AND_RETURN,
        RESET_AND_RETURN,
        RESET_SANDWICH,
    };

    void run_state(BotBaseContext& context);

    //  Returns true on success.
    bool run_traversal(BotBaseContext& context);


private:
    OCR::LanguageOCROption LANGUAGE;

    enum class Mode{
        START_ON_PLATFORM,
        START_IN_ZERO_GATE,
        MAKE_SANDWICH,
    };
    EnumDropdownOption<Mode> MODE;

    enum class Path{
        PATH0,
        PATH1,
        PATH2,
    };
    EnumDropdownOption<Path> PATH0;

    EncounterBotCommonOptions ENCOUNTER_BOT_OPTIONS;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    PlatformResetSettings PLATFORM_RESET;
    NavigatePlatformSettings NAVIGATE_TO_PLATFORM;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationsOption NOTIFICATIONS;

    SingleSwitchProgramEnvironment* m_env;

    LetsGoEncounterBotTracker* m_tracker;

    uint64_t m_iterations = 0;
    State m_state;

    //  Set to true if we should save on the first available opportunity.
    bool m_pending_save;
    enum class SavedLocation{
        NONE,
        ZERO_GATE_FLY_SPOT,
        AREA_ZERO,
    };
    SavedLocation m_last_save;

    WallClock m_last_sandwich;

    size_t m_consecutive_failures;
};





}
}
}
#endif
