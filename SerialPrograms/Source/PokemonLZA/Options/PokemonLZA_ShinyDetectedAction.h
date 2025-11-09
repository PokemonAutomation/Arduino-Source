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
#include <atomic>

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLZA{


enum class ShinySoundDetectedAction{
    STOP_PROGRAM,         // stop program at first detected shiny sound
    NOTIFY_ON_FIRST_ONLY, // notify user only on the first shiny sound, keep running the program
    NOTIFY_ON_ALL,        // notify on all shiny sounds, keep running the program
};


class ShinySoundDetectedActionOption : public GroupOption, public ConfigOption::Listener{
public:
    ShinySoundDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay,
        ShinySoundDetectedAction default_action
    );

    // Handle shiny sound according to ACTION.
    // May log the sound, take a video and send notification.
    // Return whether to stop the program according to ACTION.
    bool on_shiny_sound(
        ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
        size_t current_count,
        float error_coefficient
    );

    // Assuming a shiny sound is detected, this function logs and sends a notification.
    // This function is called by `on_shiny_sound()`.
    void send_shiny_sound_notification(
        ProgramEnvironment& env, VideoStream& stream,
        float error_coefficient
    );

    StaticTextOption DESCRIPTION;
    EnumDropdownOption<ShinySoundDetectedAction> ACTION;
    BooleanCheckBoxOption TAKE_VIDEO;
    MillisecondsOption SCREENSHOT_DELAY;
    EventNotificationOption NOTIFICATIONS;

    StaticTextOption NOTES;
};





class ShinySoundHandler{
public:
    ShinySoundHandler(ShinySoundDetectedActionOption& option)
        : m_option(option)
        , m_pending_video(false)
    {}

    //  Only call this from shiny inference thread.
    //  Returns true if we should exit from routine.
    bool on_shiny_sound(
        ProgramEnvironment& env,
        VideoStream& stream,
        size_t current_count,
        float error_coefficient
    );

    //  Only call this from program thread.
    void process_pending(ProControllerContext& context);


private:
    ShinySoundDetectedActionOption& m_option;

    //  Set to true by the shiny inference thread.
    //  Set to false by the program thread after it processes the pending video.
    std::atomic<bool> m_pending_video;

    WallClock m_detected_time;
};












}
}
}
#endif
