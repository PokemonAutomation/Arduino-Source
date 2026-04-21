/*  Shiny Hunt - Bench Sit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyHunt_BenchSit_H
#define PokemonAutomation_PokemonLZA_ShinyHunt_BenchSit_H

#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"
#include "PokemonLZA/Inference/PokemonLZA_DayNightStateDetector.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{




class ShinyHunt_BenchSit_Descriptor : public SingleSwitchProgramDescriptor{
public:
    ShinyHunt_BenchSit_Descriptor();

    class Stats;
    virtual std::unique_ptr<StatsTracker> make_stats() const override;
};


class ShinyHunt_BenchSit : public SingleSwitchProgramInstance{
public:
    ShinyHunt_BenchSit();

    virtual void program(SingleSwitchProgramEnvironment& env, ProControllerContext& context) override;

private:
    void check_daynight(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context
    );
    void run_rounds(
        SingleSwitchProgramEnvironment& env,
        ProControllerContext& context,
        ShinySoundHandler& shiny_sound_handler
    );

private:
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    IntegerEnumDropdownOption WALK_DIRECTION;
    MillisecondsOption WALK_FORWARD_DURATION;
    SimpleIntegerOption<uint32_t> PERIODIC_SAVE;

    GroupOption DAY_NIGHT_FILTER;
    IntegerEnumDropdownOption DAY_FILTER_MODE;
    SimpleIntegerOption<uint32_t> PERIODIC_TIME_CHECK;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;

    std::unique_ptr<DayNightStateDetector> m_day_night_detector;
    bool should_run_based_on_day_night(const ImageViewRGB32& frame, VideoOverlay& overlay);

    DayNightState m_cached_time = DayNightState::DAY;
    bool m_cached_time_initialized = false;
    uint32_t m_rounds_since_time_check = 0;
};





}
}
}
#endif
