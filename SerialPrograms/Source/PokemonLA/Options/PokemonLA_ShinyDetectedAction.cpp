/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include "CommonFramework/Exceptions/ProgramFinishedException.h"
#include "CommonFramework/Options/Environment/ThemeSelectorOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
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



ShinyDetectedActionOption::ShinyDetectedActionOption(
    std::string label, std::string description,
    std::string default_delay_ticks,
    ShinyDetectedAction default_action
)
    : GroupOption(std::move(label), LockWhileRunning::LOCKED)
    , DESCRIPTION(std::move(description))
    , ACTION(
        "<b>Shiny Detected Action:</b>",
        {
            {ShinyDetectedAction::IGNORE,                   "ignore",       "Ignore the shiny. Do not stop the program."},
            {ShinyDetectedAction::STOP_PROGRAM,             "stop",         "Stop program. Align camera for a screenshot. Then go Home."},
            {ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM,  "video+stop",   "Stop program. Align camera for a screenshot + video. Then go Home."},
        },
        LockWhileRunning::LOCKED,
        default_action
    )
//    , STOP_PROGRAM("<b>Stop Program:</b><br>Stop program and go Home if it hears a shiny.", true)
//    , TAKE_VIDEO("<b>Take Video:</b><br>Take a video if a shiny is heard.", true)
    , SCREENSHOT_DELAY(
        "<b>Screenshot Delay:</b><br>"
        "Align the camera, then wait this long before taking a screenshot + video of the shiny.<br>"
        "Set to zero to skip this. Don't set this too large or the shiny may run away!",
        LockWhileRunning::LOCKED,
        TICKS_PER_SECOND,
        std::move(default_delay_ticks)
    )
    , NOTIFICATIONS(this->label(), true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
{
    if (!DESCRIPTION.text().empty()){
        PA_ADD_OPTION(DESCRIPTION);
    }
    PA_ADD_OPTION(ACTION);
//    PA_ADD_OPTION(STOP_PROGRAM);
//    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(SCREENSHOT_DELAY);
}
bool ShinyDetectedActionOption::stop_on_shiny() const{
    return ACTION != ShinyDetectedAction::IGNORE;
}






bool on_shiny_callback(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options,
    float error_coefficient
){
    {
        std::ostringstream ss;
        ss << "Detected Shiny Sound! (error coefficient = " << error_coefficient << ")";
        console.log(ss.str(), COLOR_BLUE);

        //  If we're not ignoring the shiny, return now. Actions will be deferred
        //  until after the session ends.
        ShinyDetectedAction action = options.ACTION;
        if (action != ShinyDetectedAction::IGNORE){
            return true;
        }
    }

    console.log("Ignoring shiny per user settings...", COLOR_RED);

    std::vector<std::pair<std::string, std::string>> embeds;

    std::ostringstream ss;
    ss << "Error Coefficient: " << error_coefficient << "\n(Shiny may not be visible on the screen.)";
    embeds.emplace_back("Detection Results:", ss.str());

    send_program_notification(
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        console.video().snapshot(), true
    );
    return false;
}
void on_shiny_sound(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    ShinyDetectedActionOption& options,
    float error_coefficient
){
    std::vector<std::pair<std::string, std::string>> embeds;

    std::ostringstream ss;
    ss << "Error Coefficient: ";
    ss << error_coefficient;
    ss << "\n(Shiny may not be visible on the screen.)";
    embeds.emplace_back("Detection Results:", ss.str());

//    pbf_press_button(context, BUTTON_ZL, 20, options.SCREENSHOT_DELAY);
    pbf_mash_button(context, BUTTON_ZL, options.SCREENSHOT_DELAY);
    context.wait_for_all_requests();

    send_program_notification(
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Detected Shiny Sound",
        embeds, "",
        console.video().snapshot(), true
    );

    ShinyDetectedAction action = options.ACTION;
    if (action == ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM){
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    context.wait_for_all_requests();
    throw ProgramFinishedException();
}

void on_match_found(
    ProgramEnvironment& env, ConsoleHandle& console, BotBaseContext& context,
    ShinyDetectedActionOption& options, bool stop_program
){
    std::vector<std::pair<std::string, std::string>> embeds;

    pbf_mash_button(context, BUTTON_ZL, options.SCREENSHOT_DELAY);
    context.wait_for_all_requests();

    send_program_notification(
        env, options.NOTIFICATIONS,
        Pokemon::COLOR_STAR_SHINY,
        "Match Found",
        embeds, "",
        console.video().snapshot(), true
    );

    ShinyDetectedAction action = options.ACTION;
    if (action == ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM){
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    if (stop_program){
        console.log("Stopping...");
//        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        context.wait_for_all_requests();
        throw ProgramFinishedException();
    }

    console.log("Continuing...");
}












}
}
}
