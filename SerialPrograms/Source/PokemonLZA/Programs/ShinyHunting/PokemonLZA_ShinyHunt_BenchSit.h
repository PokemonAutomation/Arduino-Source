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
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/Notifications/EventNotificationsTable.h"
#include "CommonFramework/ImageTools/ImageBoxes.h"
#include "NintendoSwitch/NintendoSwitch_SingleSwitchProgram.h"
#include "PokemonLA/Options/PokemonLA_ShinyDetectedAction.h"
#include "PokemonLZA/Options/PokemonLZA_ShinyDetectedAction.h"
#include "PokemonLZA/Inference/PokemonLZA_DayNightStateDetector.h"
#include "PokemonLZA/Inference/PokemonLZA_WeatherDetector.h"

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
    bool should_run_based_on_day_night(const ImageViewRGB32& frame);
    bool should_run_based_on_weather(const ImageViewRGB32& frame);
    PokemonLA::ShinyRequiresAudioText SHINY_REQUIRES_AUDIO;

    IntegerEnumDropdownOption WALK_DIRECTION;
    MillisecondsOption WALK_FORWARD_DURATION;
    SimpleIntegerOption<uint32_t> PERIODIC_SAVE;

    GroupOption DAY_NIGHT_FILTER;
    IntegerEnumDropdownOption FILTER_MODE;

    GroupOption WEATHER_FILTER;
    IntegerEnumDropdownOption WEATHER_FILTER_MODE;

    ShinySoundDetectedActionOption SHINY_DETECTED;

    EventNotificationOption NOTIFICATION_STATUS;
    EventNotificationsOption NOTIFICATIONS;
};





}
}
}
#endif
