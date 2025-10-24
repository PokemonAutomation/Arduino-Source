/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLZA_ShinyDetectedAction_H

#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options//TimeDurationOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLZA{


enum class ShinySoundDetectedAction{
    STOP_PROGRAM,
    NOTIFY_ON_FIRST_ONLY,
    NOTIFY_ON_ALL,
};


class ShinySoundDetectedActionOption : public GroupOption, public ConfigOption::Listener{
public:
    ShinySoundDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay,
        ShinySoundDetectedAction default_action
    );

    bool on_shiny_sound(
        ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
        size_t current_count,
        float error_coefficient
    );

    StaticTextOption DESCRIPTION;
    EnumDropdownOption<ShinySoundDetectedAction> ACTION;
    BooleanCheckBoxOption TAKE_VIDEO;
    MillisecondsOption SCREENSHOT_DELAY;
    EventNotificationOption NOTIFICATIONS;

    StaticTextOption NOTES;
};





}
}
}
#endif
