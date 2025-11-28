/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_PokemonLZA_ShinyDetectedAction_H
#define PokemonAutomation_PokemonLZA_ShinyDetectedAction_H

#include <atomic>
#include "Common/Cpp/Options/GroupOption.h"
#include "Common/Cpp/Options/StaticTextOption.h"
#include "Common/Cpp/Options/BooleanCheckBoxOption.h"
//#include "Common/Cpp/Options/SimpleIntegerOption.h"
#include "Common/Cpp/Options//TimeDurationOption.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch/Controllers/NintendoSwitch_ProController.h"

namespace PokemonAutomation{
    class ProgramEnvironment;
namespace NintendoSwitch{
namespace PokemonLZA{


enum class ShinySoundDetectedAction{
    STOP_PROGRAM,         // stop program at first detected shiny sound
    NOTIFY_ON_FIRST_ONLY, // notify user only on the first shiny sound, keep running the program
    NOTIFY_ON_ALL,        // notify on all shiny sounds, keep running the program
    NO_NOTIFICATIONS      // no notifications, still track shiny sounds
};


class ShinySoundDetectedActionOption : public GroupOption, public ConfigOption::Listener{
public:
    ShinySoundDetectedActionOption(
        std::string label, std::string description,
        std::string default_delay,
        ShinySoundDetectedAction default_action
    );

    // Handle shiny sound according to ACTION.
    // May log the sound, take a video on Switch and send notification.
    // Return whether to stop the program according to ACTION.
    // NOTE:
    // Since this function may file button presses to record a video on Switch, it must be
    // in the program thread that allows sending button presses. Don't call this function on
    // audio callback thread!
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





// Thread-safe handler for shiny sound detection and video capture.
//
// THREADING MODEL:
// - on_shiny_sound() is called from the shiny inference thread (audio detector callback)
// - process_pending() is called from the program thread (sending button presses)
class ShinySoundHandler{
public:
    ShinySoundHandler(ShinySoundDetectedActionOption& option)
        : m_option(option)
        , m_pending_video(false)
    {}

    // Must be called by shiny inference thread (audio detector callback).
    // Records detection time and sets pending flag using memory_order_release.
    // Returns true if the program should exit (ACTION == STOP_PROGRAM).
    bool on_shiny_sound(
        ProgramEnvironment& env,
        VideoStream& stream,
        size_t current_count,
        float error_coefficient
    );

    // Must be called from program thread (sending button presses).
    // According to option, takes video after waiting for the appropriate delay
    // based on elapsed time.
    void process_pending(ProControllerContext& context);

private:
    ShinySoundDetectedActionOption& m_option;

    // SYNCHRONIZATION STRATEGY:
    // Uses the atomic flag m_pending_video with release-acquire memory ordering to synchronize
    // access to the non-atomic m_detected_time:
    //
    //   Inference Thread:                      Program Thread:
    //   on_shiny_sound()                       process_pending()
    //   ─────────────────                      ───────────────
    //   m_detected_time = now;
    //   m_pending_video.store(true, RELEASE) ──────┐
    //                                              │ Synchronizes-with
    //                                              ↓
    //                                   m_pending_video.load(ACQUIRE)
    //                                   read m_detected_time safely and process video recording
    //                                   m_pending_video.store(false, RELEASE)
    //
    // m_pending_video:
    // - Set to true with memory_order_release by inference thread (signals need for recording a
    //   video and publishes timestamp)
    // - Read with memory_order_acquire by program thread (reads whether to record a video and
    //   gets published timestamp)
    // - Set to false with memory_order_release by program thread after processing
    std::atomic<bool> m_pending_video;

    // Time when shiny was detected (non-atomic, protected by m_pending_video flag).
    // THREAD SAFETY: Only written by inference thread before setting m_pending_video=true.
    // Only read by program thread after observing m_pending_video=true.
    WallClock m_detected_time;
};












}
}
}
#endif
