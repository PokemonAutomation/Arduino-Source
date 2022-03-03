/*  Shiny Detected Action
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/StatsTracking.h"
#include "CommonFramework/Tools/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/Tools/InterruptableCommands.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "PokemonLA/PokemonLA_Settings.h"
#include "PokemonLA_ShinyDetectedAction.h"

namespace PokemonAutomation{
namespace NintendoSwitch{
namespace PokemonLA{



ShinyDetectedActionOption::ShinyDetectedActionOption()
    : GroupOption("Shiny Detected Actions")
//    , STOP_PROGRAM("<b>Stop Program:</b><br>Stop the program and go Home if the program hears a shiny.", true)
//    , TAKE_VIDEO("<b>Take Video:</b><br>Take a video if a shiny is heard.", true)
    , DESCRIPTION("<font color=\"blue\">Shiny detection uses sound. Make sure you have the correct audio input set.</font>")
    , ACTION(
        "<b>Shiny Detected Action:</b>",
        {
            "Ignore the shiny. (continue with program)",
            "Go Home and stop program.",
            "Take video. Then go Home and stop program.",
        },
        2
    )
    , VIDEO_DELAY(
        "<b>Video Delay:</b><br>"
        "If taking a video, wait this long before saving video. "
        "Don't wait too long or the shiny may run away!",
        "2 * TICKS_PER_SECOND"
    )
    , NOTIFICATIONS("Shiny Detected", true, true, ImageAttachmentMode::JPG, {"Notifs", "Showcase"})
{
    PA_ADD_OPTION(DESCRIPTION);
//    PA_ADD_OPTION(STOP_PROGRAM);
//    PA_ADD_OPTION(TAKE_VIDEO);
    PA_ADD_OPTION(ACTION);
    PA_ADD_OPTION(VIDEO_DELAY);
}
bool ShinyDetectedActionOption::stop_on_shiny() const{
    return ACTION != 0;
}
bool ShinyDetectedActionOption::do_nothing() const{
    return ACTION == 0;
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
        console, option.NOTIFICATIONS, Color(0xffff99),
        env.program_info(),
        "Detected Possible Shiny",
        embeds,
        screenshot, true
    );

    return (ShinyDetectedAction)(size_t)option.ACTION != ShinyDetectedAction::IGNORE;
}
#endif


void on_shiny(
    ProgramEnvironment& env, ConsoleHandle& console,
    ShinyDetectedActionOption& options, QImage screenshot
){
    std::vector<std::pair<QString, QString>> embeds;
    const StatsTracker* stats = env.stats();
    if (stats){
        std::string str = stats->to_str();
        if (!str.empty()){
            embeds.emplace_back("Session Stats", QString::fromStdString(str));
        }
    }
    embeds.emplace_back("Detection Method", "Audio. (Shiny may not be visible on the screen.)");

    send_program_notification(
        console, options.NOTIFICATIONS, Color(0xffff99),
        env.program_info(),
        "Detected Possible Shiny",
        embeds,
        screenshot, true
    );

    switch ((ShinyDetectedAction)(size_t)options.ACTION){
    case ShinyDetectedAction::IGNORE:
        return;
    case ShinyDetectedAction::STOP_PROGRAM:
        pbf_press_button(console, BUTTON_ZL, 20, options.VIDEO_DELAY);
//        pbf_wait(console, options.VIDEO_DELAY);
        pbf_press_button(console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        throw ShinyDetectedException(std::move(screenshot));
    case ShinyDetectedAction::TAKE_VIDEO_STOP_PROGRAM:
        pbf_press_button(console, BUTTON_ZL, 20, options.VIDEO_DELAY);
//        pbf_wait(console, options.VIDEO_DELAY);
        pbf_press_button(console, BUTTON_CAPTURE, 2 * TICKS_PER_SECOND, 0);
        pbf_press_button(console, BUTTON_HOME, 20, GameSettings::instance().GAME_TO_HOME_DELAY);
        throw ShinyDetectedException(std::move(screenshot));
    }
}





}
}
}
