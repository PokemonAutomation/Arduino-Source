/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
//#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



ShinySoundDetectedActionOption::ShinySoundDetectedActionOption(
    std::string label, std::string description,
    std::string default_delay,
    ShinySoundDetectedAction default_action
)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , ACTION(
        "<b>Action:</b>",
        {
            {ShinySoundDetectedAction::STOP_PROGRAM,            "stop",         "Stop program and go Home. Send notification."},
            {ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY,    "notify-first", "Keep running. Notify on first shiny sound only."},
//            {ShinySoundDetectedAction::NOTIFY_ON_ALL,           "notify-all",   "Keep running. Notify on all shiny sounds."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        default_action
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , SCREENSHOT_DELAY(
        "<b>Video Delay:</b><br>"
        "Wait this long before taking a video of the shiny.<br>"
        "Don't set this too large or the shiny may run away!",
        LockMode::UNLOCK_WHILE_RUNNING,
        std::move(default_delay)
    )
    , NOTIFICATIONS(
        this->label(),
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
    , NOTES(
        "<font color=\"red\">"
        "The shiny sound is not a reliable measure of shinies encountered:<br>"
        "1. The sound only plays on a smaller radius than the spawn radius, so the vast majority of shinies are inaudible. "
        "You will still need to manually run around to see if any shinies spawned out-of-range. "
        "<br>"
        "2. Secondly, it may play multiple times for the same shiny, so it may overcount. "
        "If a shiny spawns next to you, it may play the sound on every reset afterwards. <\font>"
    )
{
    if (!DESCRIPTION.text().empty()){
        PA_ADD_STATIC(DESCRIPTION);
    }
    PA_ADD_OPTION(ACTION);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(SCREENSHOT_DELAY);

    PA_ADD_STATIC(NOTES);
}

bool ShinySoundDetectedActionOption::on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    size_t current_count,
    float error_coefficient
){
    ShinySoundDetectedAction action = ACTION;

    if (action == ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY && current_count > 1){
        return false;
    }

    if (TAKE_VIDEO){
        context.wait_for(SCREENSHOT_DELAY);
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    send_shiny_sound_notification(env, stream, error_coefficient);

    return action == ShinySoundDetectedAction::STOP_PROGRAM;
}


void ShinySoundDetectedActionOption::send_shiny_sound_notification(
    ProgramEnvironment& env, VideoStream& stream, float error_coefficient
){
    {
        std::ostringstream ss;
        ss << "Detected Shiny Sound! (error coefficient = " << error_coefficient << ")";
        stream.log(ss.str(), COLOR_BLUE);
    }

    std::vector<std::pair<std::string, std::string>> embeds;
    {
        std::ostringstream ss;
        ss << "Error Coefficient: ";
        ss << error_coefficient;
        ss << "\n(Shiny may not be visible on the screen.)";
        embeds.emplace_back("Detection Results:", ss.str());
    }

    send_program_notification(
        env, NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        stream.video().snapshot(), true
    );
}






bool ShinySoundHandler::on_shiny_sound(
    ProgramEnvironment& env,
    VideoStream& stream,
    size_t current_count,
    float error_coefficient
){
    WallClock now = current_time();

    ShinySoundDetectedAction action = m_option.ACTION;

    if (action == ShinySoundDetectedAction::NOTIFY_ON_FIRST_ONLY && current_count > 1){
        return false;
    }

    m_option.send_shiny_sound_notification(env, stream, error_coefficient);

    if (m_pending_video.load(std::memory_order_acquire)){
        stream.log("Back-to-back shiny sounds. Suppressing video.", COLOR_RED);
    }else{
        m_detected_time = now;
        m_pending_video.store(true, std::memory_order_release);
    }

    return action == ShinySoundDetectedAction::STOP_PROGRAM;
}

void ShinySoundHandler::process_pending(ProControllerContext& context){
    if (!m_pending_video.load(std::memory_order_acquire)){
        return;
    }
    if (!m_option.TAKE_VIDEO){
        m_pending_video.store(false, std::memory_order_release);
        return;
    }

    //  Calculate elapsed time since shiny detection.
    WallDuration elapsed = current_time() - m_detected_time;
    auto elapsed_ms = std::chrono::duration_cast<Milliseconds>(elapsed);

    //  Calculate remaining time to wait.
    Milliseconds requested_delay = m_option.SCREENSHOT_DELAY.get();
    if (requested_delay > elapsed_ms){
        context.wait_for(requested_delay - elapsed_ms);
    }

    //  Otherwise, take screenshot immediately (no additional wait needed)
    pbf_press_button(context, BUTTON_CAPTURE, 2000ms, 0ms);

    //  Now we can unlock.
    m_pending_video.store(false, std::memory_order_release);
}


















}
}
}
