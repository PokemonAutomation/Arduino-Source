/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLA_ShinyDetectedAction_H

#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
#include "Common/Cpp/Options/EnumDropdownOption.h"
#include "Common/Cpp/Options/TimeDurationOption.h"
#include "Common/Cpp/Options/GroupOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

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




enum class OverworldShinyDetectedAction{
    IGNORE,
    STOP_PROGRAM,
    TAKE_VIDEO_STOP_PROGRAM,
};


class OverworldShinyDetectedActionOption : public GroupOption{
public:
    OverworldShinyDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay,
        OverworldShinyDetectedAction default_action = OverworldShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM
    );

    bool stop_on_shiny() const;

    StaticTextOption DESCRIPTION;
    EnumDropdownOption<OverworldShinyDetectedAction> ACTION;
//    BooleanCheckBoxOption STOP_PROGRAM;
//    BooleanCheckBoxOption TAKE_VIDEO;
    MillisecondsOption SCREENSHOT_DELAY0;
    EventNotificationOption NOTIFICATIONS;
};
class BattleMatchActionOption : public GroupOption{
public:
    BattleMatchActionOption(
        std::string label, std::string description,
        std::string default_delay
    );

    bool stop_on_shiny() const;

    StaticTextOption DESCRIPTION;
    BooleanCheckBoxOption TAKE_VIDEO;
    EventNotificationOption NOTIFICATIONS;
};



//  Call this inside the ShinySoundDetector callback.
//  Returns true if session should stop.
bool on_shiny_callback(
    ProgramEnvironment& env, VideoStream& stream,
    OverworldShinyDetectedActionOption& options,
    float error_coefficient
);

//  Call this after the session ends. Only if the session stopped on the shiny.
void on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    OverworldShinyDetectedActionOption& options,
    float error_coefficient
);

// Alternative for matches (shiny/alphas) not found by sound.
void on_battle_match_found(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    BattleMatchActionOption& options,
    bool allow_notification
);








}
}
}
#endif
