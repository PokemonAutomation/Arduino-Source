/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLA_ShinyDetectedAction_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeExpressionOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_Controller.h"

namespace PokemonAutomation{
    class EventNotificationOption;
    class StatsTracker;
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLA{


class ShinyRequiresAudioText : public StaticTextOption{
public:
    ShinyRequiresAudioText();
};



class ShinySoundDetector;


class ShinyStatIncrementer{
public:
    virtual void add_shiny() = 0;
};




enum class ShinyDetectedAction{
    IGNORE,
    STOP_PROGRAM,
    TAKE_VIDEO_STOP_PROGRAM,
};


class ShinyDetectedActionOption : public GroupOption{
public:
    ShinyDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay_ticks,
        ShinyDetectedAction default_action = ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM
    );

    bool stop_on_shiny() const;

    StaticTextOption DESCRIPTION;
    EnumDropdownOption<ShinyDetectedAction> ACTION;
//    BooleanCheckBoxOption STOP_PROGRAM;
//    BooleanCheckBoxOption TAKE_VIDEO;
    TimeExpressionOption<uint16_t> SCREENSHOT_DELAY;
    EventNotificationOption NOTIFICATIONS;
};



//  Call this inside the ShinySoundDetector callback.
//  Returns true if session should stop.
bool on_shiny_callback(
    ProgramEnvironment& env, VideoStream& stream,
    ShinyDetectedActionOption& options,
    float error_coefficient
);

//  Call this after the session ends. Only if the session stopped on the shiny.
void on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    ShinyDetectedActionOption& options,
    float error_coefficient
);

// Alternative for matches (shiny/alphas) not found by sound.
void on_match_found(
    ProgramEnvironment& env, VideoStream& stream, SwitchControllerContext& context,
    ShinyDetectedActionOption& options, bool is_match
);








}
}
}
#endif
