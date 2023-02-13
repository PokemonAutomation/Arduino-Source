/*  Shiny Hunt - Area Zero Platform
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonSV_ShinyHuntAreaZyroPlatform_H
#define PokemonAutomation_PokemonSV_ShinyHuntAreaZyroPlatform_H

#include <functional>
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "NintendoSwitch/Options/NintendoSwitch_GoHomeWhenDoneOption.h"
#include "PokemonSV_AreaZyroPlatform.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonSV{

class EncounterWatcher;
class LetsGoKillSoundDetector;




class ShinyHuntAreaZyroPlatform_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHuntAreaZyroPlatform_Descriptor();

    struct Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHuntAreaZyroPlatform : public SingleSwitchProgramInstance{
public:
    ShinyHuntAreaZyroPlatform();

    virtual void program(SingleSwitchProgramEnvironment& env, BotBaseContext& context) override;

private:
    enum class State{
        TRAVERSAL,
        INSIDE_GATE_AND_RETURN,
        LEAVE_AND_RETURN,
        RESET_AND_RETURN,
    };

    void run_state(BotBaseContext& context);
    void on_shiny_encounter(BotBaseContext& context, EncounterWatcher& encounter_watcher);

    void run_traversal(BotBaseContext& context);
    bool clear_in_front(
        BotBaseContext& context,
        bool throw_ball_if_bubble,
        std::function<void(BotBaseContext& context)>&& command
    );

    void run_path0(BotBaseContext& context);
    void run_path1(BotBaseContext& context);
    void run_path2(BotBaseContext& context);



private:
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

    PlatformResetSettings PLATFORM_RESET;
    NavigatePlatformSettings NAVIGATE_TO_PLATFORM;

    EventNotificationOption NOTIFICATION_STATUS_UPDATE;
    EventNotificationOption NOTIFICATION_SHINY;
    EventNotificationsOption NOTIFICATIONS;

    SingleSwitchProgramEnvironment* m_env;
    LetsGoKillSoundDetector* m_kill_watcher;
    uint64_t m_iterations = 0;
    State m_state;
    WallClock m_last_platform_reset;
    size_t m_consecutive_failures;
};





}
}
}
#endif
