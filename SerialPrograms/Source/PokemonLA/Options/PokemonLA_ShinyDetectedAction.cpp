/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <sstream>
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "Pokemon/Pokemon_Notification.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA/Inference/PokemonLA_ShinySoundDetector.h"
#include "PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinyDetectedActionOption::ShinyDetectedActionOption()
    : GroupOption("Shiny Detected Actions")
    , DESCRIPTION("<font color=\"blue\">Shiny detection uses sound. Make sure you have the correct audio input set.</font>")
    , ACTION(
        "<b>Shiny Detected Action:</b><br><br>"
        "If set to ignore the shiny, the notification may be delayed depending on the program. "
        "Furthermore, the screenshot will be from the moment the shiny sound was detected. "
        "No attempt will be made to align the camera or wait for the shiny to come into view.<br><br>"
        "This limitation is because most programs cannot be interrupted without completely stopping the program. "
        "So you must either completely ignore the shiny or completely stop the program.",
        {
            "Ignore the shiny. Do not stop the program.",
            "Stop program. Align camera for a screenshot. Then go Home.",
            "Stop program. Align camera for a screenshot + video. Then go Home.",
        },
        2
    )
//    , STOP_PROGRAM("<b>Stop Program:</b><br>Stop program and go Home if it hears a shiny.", true)
//    , TAKE_VIDEO("<b>Take Video:</b><br>Take a video if a shiny is heard.", true)
    , SCREENSHOT_DELAY(
        "<b>Screenshot Delay:</b><br><br>"
        "Align the camera, then wait this long before taking a screenshot + video of the shiny.<br>"
        "If set to zero or a small value, it will not be able to fully align the camera.<br>"
        "(Don't wait too long or the shiny may run away!)",
        "2 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS("Shiny Detected", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
{
    PA_ADD_OPTION(DESCRIPTION);
    PA_ADD_OPTION(ACTION);
//    PA_ADD_OPTION(STOP_PROGRAM);
//    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(SCREENSHOT_DELAY);
}
bool ShinyDetectedActionOption::stop_on_shiny() const{
    return ACTION != (size_t)ShinyDetectedAction::IGNORE;
}


#if 0
bool run_on_shiny(
    const BotBaseContext& context,
    const ShinyDetectedActionOption& option
){
    switch ((ShinyDetectedAction)(size_t)option.ACTION){
    case ShinyDetectedAction::IGNORE:
        return false;
    case ShinyDetectedAction::STOP_PROGRAM:
        pbf_wait(context, option.VIDEO_DELAY);
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        return true;
    case ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM:
        pbf_wait(context, option.VIDEO_DELAY);
        pbf_press_button(context, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
        pbf_press_button(context, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        return true;
    }
    return false;
}


bool run_on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    AsyncCommandSession& command_session,
    ShinyDetectedActionOption& option,
    const StatsTracker* session_stats
){
    QImage screenshot = console.video().snapshot();
    if (!option.do_nothing()){
        command_session.dispatch([&](const BotBaseContext& context){
            run_on_shiny(context, option);
        });
        command_session.wait();
    }

    std::vector<std::pair<QString, QString>> embeds;
    if (session_stats){
        std::string str = session_stats->to_str();
        if (!str.empty()){
            embeds.emplace_back("Session Stats", QString::fromStdString(str));
        }
    }

    send_program_notification(
        console, option.NOTIFICATIONS, Color(COLOR_STAR_SHINY),
        env.program_info(),
        "Detected Shiny Sound",
        embeds,
        screenshot, true
    );

    return (ShinyDetectedAction)(size_t)option.ACTION != ShinyDetectedAction::IGNORE;
}
#endif


void on_shiny_sound(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options,
    const ShinySoundResults& results
){
    std::vector<std::pair<QString, QString>> embeds;
    const StatsTracker* stats = env.stats();
    if (stats){
        std::string str = stats->to_str();
        if (!str.empty()){
            embeds.emplace_back("Session Stats", QString::fromStdString(str));
        }
    }

    std::stringstream ss;
    ss << "Error Coefficient: ";
    ss << results.error_coefficient;
    ss << "\n(Shiny may not be visible on the screen.)";

    embeds.emplace_back("Detection Results", QString::fromStdString(ss.str()));

    ShinyDetectedAction action = (ShinyDetectedAction)(size_t)options.ACTION;

    if (action == ShinyDetectedAction::IGNORE){
        send_program_notification(
            console, options.NOTIFICATIONS, Pokemon::COLOR_STAR_SHINY,
            env.program_info(),
            "Detected Shiny Sound",
            embeds,
            results.screenshot, true
        );
        return;
    }

//    pbf_press_button(console, BUTTON_ZL, 20, options.SCREENSHOT_DELAY);
    pbf_mash_button(console, BUTTON_ZL, options.SCREENSHOT_DELAY);
    console.botbase().wait_for_all_requests();

    QImage screen = console.video().snapshot();
    send_program_notification(
        console, options.NOTIFICATIONS, Pokemon::COLOR_STAR_SHINY,
        env.program_info(),
        "Detected Shiny Sound",
        embeds,
        screen, true
    );

    if (action == ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM){
        pbf_press_button(console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
    }

    pbf_press_button(console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
    throw OperationCancelledException();
}





}
}
}
