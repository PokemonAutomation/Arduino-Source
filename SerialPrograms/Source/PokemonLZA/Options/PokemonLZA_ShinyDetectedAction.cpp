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
            {ShinySoundDetectedAction::NOTIFY_ON_ALL,           "notify-all",   "Keep running. Notify on all shiny sounds."},
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
        "<b>Screenshot Delay:</b><br>"
        "Wait this long before taking a screenshot + video of the shiny.<br>"
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

    {
        std::ostringstream ss;
        ss << "Detected Shiny Sound! (error coefficient = " << error_coefficient << ")";
        stream.log(ss.str(), COLOR_BLUE);
    }

    if (TAKE_VIDEO){
        context.wait_for(SCREENSHOT_DELAY);
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
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

    return action == ShinySoundDetectedAction::STOP_PROGRAM;
}






















}
}
}
