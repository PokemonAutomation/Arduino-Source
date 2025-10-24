/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonLZA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLZA{



ShinyDetectedActionOption::ShinyDetectedActionOption(
    std::string label, std::string description,
    std::string default_delay,
    ShinyDetectedActionType default_action
)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , ACTION(
        "<b>Action:</b>",
        {
            {ShinyDetectedActionType::IGNORE,           "ignore",       "Ignore the shiny. Do not stop the program."},
            {ShinyDetectedActionType::STOP_PROGRAM,     "stop",         "Stop program and go Home."},
            {ShinyDetectedActionType::STOP_AFTER_COUNT, "stop-count",   "Stop program only after X shinies are found."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        default_action
    )
    , MAX_COUNT(
        "<b>Max Detections:</b><br>"
        "Stop the program after this many shinies are detected.",
        LockMode::UNLOCK_WHILE_RUNNING,
        10
    )
    , TAKE_VIDEO(
        "<b>Take Video:</b>",
        LockMode::UNLOCK_WHILE_RUNNING,
        true
    )
    , SCREENSHOT_DELAY(
        "<b>Screenshot Delay:</b><br>"
        "Wait this long before taking a screenshot + video of the shiny.<br>"
        "Set to zero to skip this. Don't set this too large or the shiny may run away!",
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
        "The shiny sound is not a reliable measure of shinies encountered. "
        "First, the sound only plays on a smaller radius than the spawn radius, so the vast majority of shinies are inaudible. "
        "Secondly, it may play multiple times for the same shiny, so it may overcount. "
        "You will still need to manually run around to see if any shinies spawned out-of-range.<\font>"
    )
{
    if (!DESCRIPTION.text().empty()){
        PA_ADD_STATIC(DESCRIPTION);
    }
    PA_ADD_OPTION(ACTION);
    PA_ADD_OPTION(MAX_COUNT);
    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(SCREENSHOT_DELAY);

    PA_ADD_STATIC(NOTES);

    ShinyDetectedActionOption::on_config_value_changed(this);

    ACTION.add_listener(*this);
}
ShinyDetectedActionOption::~ShinyDetectedActionOption(){
    ACTION.remove_listener(*this);
}
void ShinyDetectedActionOption::on_config_value_changed(void* object){
    MAX_COUNT.set_visibility(
        ACTION == ShinyDetectedActionType::STOP_AFTER_COUNT
            ? ConfigOptionState::ENABLED
            : ConfigOptionState::HIDDEN
    );
}

bool ShinyDetectedActionOption::stop_on_shiny(uint8_t current_count) const{
    switch (ACTION){
    case ShinyDetectedActionType::IGNORE:
        return false;
    case ShinyDetectedActionType::STOP_PROGRAM:
        return true;
    case ShinyDetectedActionType::STOP_AFTER_COUNT:
        return current_count >= MAX_COUNT;
    default:
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid shiny action enum type.");
    }
}








bool on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    ShinyDetectedActionOption& options,
    uint8_t current_count,
    float error_coefficient
){
    {
        std::ostringstream ss;
        ss << "Detected Shiny Sound! (error coefficient = " << error_coefficient << ")";
        stream.log(ss.str(), COLOR_BLUE);
    }

    if (options.TAKE_VIDEO){
        context.wait_for(options.SCREENSHOT_DELAY);
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
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        stream.video().snapshot(), true
    );

    return options.stop_on_shiny(current_count);
}


















}
}
}
