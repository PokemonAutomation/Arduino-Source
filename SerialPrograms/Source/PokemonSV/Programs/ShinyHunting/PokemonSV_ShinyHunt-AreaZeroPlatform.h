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
#include "PokemonSV/Options/PokemonSV_EncounterActionsTable.h"
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
    };

    void run_state(BotBaseContext& context);

    void run_traversal(BotBaseContext& context);

    void run_path0(BotBaseContext& context);
    void run_path1(BotBaseContext& context);
    void run_path2(BotBaseContext& context);


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

    BooleanCheckBoxOption VIDEO_ON_SHINY;
    GoHomeWhenDoneOption GO_HOME_WHEN_DONE;

    EncounterActionsTable ACTIONS_TABLE;
    PlatformResetSettings PLATFORM_RESET;
    NavigatePlatformSettings NAVIGATE_TO_PLATFORM;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_NONSHINY;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    SingleSwitchProgramEnvironment* m_env;

    LetsGoEncounterBotTracker* m_tracker;

    uint64_t m_iterations = 0;
    State m_state;
    size_t m_consecutive_failures;
};





}
}
}
#endif
