/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <sstream>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinyRequiresAudioText::ShinyRequiresAudioText()
    : StaticTextOption(
        html_color_text(
            "<font size=4><b>Shiny detection uses sound. Make sure you have the correct audio input set.</b></font>",
            COLOR_BLUE
        )
    )
{}




OverworldShinyDetectedActionOption::OverworldShinyDetectedActionOption(
    std::string label, std::string description,
    std::string default_delay,
    OverworldShinyDetectedAction default_action
)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , ACTION(
        "<b>Shiny Detected Action:</b>",
        {
            {OverworldShinyDetectedAction::IGNORE,                  "ignore",       "Ignore the shiny. Do not stop the program."},
            {OverworldShinyDetectedAction::STOP_PROGRAM,            "stop",         "Stop program. Align camera for a screenshot. Then go Home."},
            {OverworldShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM, "video+stop",   "Stop program. Align camera for a screenshot + video. Then go Home."},
        },
        LockMode::UNLOCK_WHILE_RUNNING,
        default_action
    )
//    , STOP_PROGRAM("<b>Stop Program:</b><br>Stop program and go Home if it hears a shiny.", true)
//    , TAKE_VIDEO("<b>Take Video:</b><br>Take a video if a shiny is heard.", true)
    , SCREENSHOT_DELAY0(
        "<b>Screenshot Delay:</b><br>"
        "Align the camera, then wait this long before taking a screenshot + video of the shiny.<br>"
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
{
    if (!DESCRIPTION.text().empty()){
        PA_ADD_OPTION(DESCRIPTION);
    }
    PA_ADD_OPTION(ACTION);
//    PA_ADD_OPTION(STOP_PROGRAM);
//    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(SCREENSHOT_DELAY0);
}
bool OverworldShinyDetectedActionOption::stop_on_shiny() const{
    return ACTION != OverworldShinyDetectedAction::IGNORE;
}



BattleMatchActionOption::BattleMatchActionOption(
    std::string label, std::string description,
    std::string default_delay
)
    : GroupOption(std::move(label), LockMode::UNLOCK_WHILE_RUNNING)
    , DESCRIPTION(std::move(description))
    , TAKE_VIDEO("<b>Take Video:</b>", LockMode::UNLOCK_WHILE_RUNNING, true)
    , NOTIFICATIONS(
        this->label(),
        true, true,
        ImageAttachmentMode::JPG,
        {"Notifs", "Showcase"}
    )
{
    if (!DESCRIPTION.text().empty()){
        PA_ADD_OPTION(DESCRIPTION);
    }
    PA_ADD_OPTION(TAKE_VIDEO);
}






bool on_shiny_callback(
    ProgramEnvironment& env, VideoStream& stream,
    OverworldShinyDetectedActionOption& options,
    float error_coefficient
){
    {
        std::ostringstream ss;
        ss << "Detected Shiny Sound! (error coefficient = " << error_coefficient << ")";
        stream.log(ss.str(), COLOR_BLUE);

        //  If we're not ignoring the shiny, return now. Actions will be deferred
        //  until after the session ends.
        OverworldShinyDetectedAction action = options.ACTION;
        if (action != OverworldShinyDetectedAction::IGNORE){
            return true;
        }
    }

    stream.log("Ignoring shiny per user settings...", COLOR_RED);

    std::vector<std::pair<std::string, std::string>> embeds;

    std::ostringstream ss;
    ss << "Error Coefficient: " << error_coefficient << "\n(Shiny may not be visible on the screen.)";
    embeds.emplace_back("Detection Results:", ss.str());

    send_program_notification(
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        stream.video().snapshot(), true
    );
    return false;
}
void on_shiny_sound(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    OverworldShinyDetectedActionOption& options,
    float error_coefficient
){
    std::vector<std::pair<std::string, std::string>> embeds;

    std::ostringstream ss;
    ss << "Error Coefficient: ";
    ss << error_coefficient;
    ss << "\n(Shiny may not be visible on the screen.)";
    embeds.emplace_back("Detection Results:", ss.str());

//    pbf_press_button(context, BUTTON_ZL, 20, options.SCREENSHOT_DELAY);
    pbf_mash_button(context, BUTTON_ZL, options.SCREENSHOT_DELAY0);
    context.wait_for_all_requests();

    send_program_notification(
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        stream.video().snapshot(), true
    );

    switch (options.ACTION){
    case OverworldShinyDetectedAction::IGNORE:
        break;
    case OverworldShinyDetectedAction::STOP_PROGRAM:
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
        context.wait_for_all_requests();
        throw ProgramFinishedException();
    case OverworldShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM:
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
        pbf_press_button(context, BUTTON_HOME, 160ms, GameSettings::instance().GAME_TO_HOME_DELAY0);
        context.wait_for_all_requests();
        throw ProgramFinishedException();
    }
}

void on_battle_match_found(
    ProgramEnvironment& env, VideoStream& stream, ProControllerContext& context,
    BattleMatchActionOption& options,
    bool allow_notification
){
    context.wait_for_all_requests();

    if (allow_notification){
        send_program_notification(
            env, options.NOTIFICATIONS,
            Pokemon::COLOR_STAR_SHINY,
            "Match Found",
            {}, "",
            stream.video().snapshot(), true
        );
    }

    if (options.TAKE_VIDEO){
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    stream.log("Stopping...");
    context.wait_for_all_requests();
    throw ProgramFinishedException();
}












}
}
}
