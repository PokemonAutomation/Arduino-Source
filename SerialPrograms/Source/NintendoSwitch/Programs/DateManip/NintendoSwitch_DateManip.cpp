/*  Date Manip
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"
#include "NintendoSwitch_DateManip.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


//using namespace std::chrono_literals;








DateReader::DateReader(ConsoleHandle& console)
    : m_console(console)
    , m_switch1(COLOR_RED)
    , m_switch2(COLOR_MAGENTA)
    , m_switch1_US(COLOR_YELLOW)
    , m_switch1_EU(COLOR_CYAN)
    , m_switch1_JP(COLOR_PURPLE)
    , m_switch2_US(COLOR_YELLOW)
    , m_switch2_EU(COLOR_CYAN)
    , m_switch2_JP(COLOR_PURPLE)
{}
void DateReader::make_overlays(VideoOverlaySet& items) const{
    m_switch1.make_overlays(items);
    m_switch2.make_overlays(items);

    m_switch1_US.make_overlays(items);
    m_switch1_EU.make_overlays(items);
    m_switch1_JP.make_overlays(items);
    m_switch2_US.make_overlays(items);
    m_switch2_EU.make_overlays(items);
    m_switch2_JP.make_overlays(items);
}
bool DateReader::detect(const ImageViewRGB32& screen){
    ConsoleType type = m_console.state().console_type();

    if (is_switch1(type)){
        return m_switch1.detect(screen);
    }

    if (is_switch2(type)){
        return m_switch2.detect(screen);
    }

    return false;
}


std::pair<DateFormat, DateTime> DateReader::read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen){
    if (!detect(*screen)){
        throw_and_log<OperationFailedException>(
            logger, ErrorReport::SEND_ERROR_REPORT,
            "Not on date change screen.",
            nullptr,
            std::move(screen)
        );
    }

    ConsoleType type = m_console.state().console_type();

    if (is_switch1(type)){
        switch (m_switch1.detect_date_format(*screen)){
        case DateFormat::US:
            return {DateFormat::US, m_switch1_US.read_date(logger, std::move(screen))};
        case DateFormat::EU:
            return {DateFormat::EU, m_switch1_EU.read_date(logger, std::move(screen))};
        case DateFormat::JP:
            return {DateFormat::JP, m_switch1_JP.read_date(logger, std::move(screen))};
        }
    }

    if (is_switch2(type)){
        switch (m_switch2.detect_date_format(*screen)){
        case DateFormat::US:
            return {DateFormat::US, m_switch2_US.read_date(logger, std::move(screen))};
        case DateFormat::EU:
            return {DateFormat::EU, m_switch2_EU.read_date(logger, std::move(screen))};
        case DateFormat::JP:
            return {DateFormat::JP, m_switch2_JP.read_date(logger, std::move(screen))};
        }
    }

    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unrecognized Console Type: " + std::to_string((int)type)
    );
}

void DateReader::set_date(
    const ProgramInfo& info, ConsoleHandle& console, ProControllerContext& context,
    const DateTime& date
){
    context.wait_for_all_requests();
    context.wait_for(std::chrono::milliseconds(250));

#if 0
    {
        auto snapshot = console.video().snapshot();
        if (!detect(snapshot)){
            throw_and_log<OperationFailedException>(
                console.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Expected date change menu.",
                &console,
                snapshot
            );
        }
    }
#endif

    auto snapshot = console.video().snapshot();

    if (!detect(snapshot)){
        throw_and_log<OperationFailedException>(
            console, ErrorReport::SEND_ERROR_REPORT,
            "Not on date change screen.",
            nullptr,
            snapshot
        );
    }

    ConsoleType type = m_console.state().console_type();

    if (is_switch1(type)){
        switch (m_switch1.detect_date_format(snapshot)){
        case DateFormat::US:
            m_switch1_US.set_date(info, console, context, date);
            return;
        case DateFormat::EU:
            m_switch1_EU.set_date(info, console, context, date);
            return;
        case DateFormat::JP:
            m_switch1_JP.set_date(info, console, context, date);
            return;
        }
    }

    if (is_switch2(type)){
        switch (m_switch2.detect_date_format(snapshot)){
        case DateFormat::US:
            m_switch2_US.set_date(info, console, context, date);
            return;
        case DateFormat::EU:
            m_switch2_EU.set_date(info, console, context, date);
            return;
        case DateFormat::JP:
            m_switch2_JP.set_date(info, console, context, date);
            return;
        }
    }

    throw InternalProgramError(
        nullptr,
        PA_CURRENT_FUNCTION,
        "Unrecognized Console Type: " + std::to_string((int)type)
    );


}

void change_date(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const DateTime& date
){
    Milliseconds timing_variation = context->timing_variation();
    while (true){
        context.wait_for_all_requests();

        HomeMenuWatcher home(env.console);
        DateChangeWatcher date_reader(env.console);
        int ret = wait_until(
            env.console, context, std::chrono::seconds(10),
            {
                home,
                date_reader
            }
        );
        switch (ret){
        case 0:
            home_to_date_time(env.console, context, true);
            pbf_press_button(context, BUTTON_A, 80ms + timing_variation, 240ms + timing_variation);
            context.wait_for_all_requests();
            continue;
        case 1:{
            env.log("Detected date change.");

            // Set the date
            VideoOverlaySet overlays(env.console.overlay());
            date_reader.make_overlays(overlays);
            date_reader.set_date(env.program_info(), env.console, context, date);

            //  Commit the date.
            pbf_press_button(context, BUTTON_A, 160ms + timing_variation, 340ms + timing_variation);

            //  Re-enter the game.
            pbf_press_button(context, BUTTON_HOME, 160ms + timing_variation, ConsoleSettings::instance().SETTINGS_TO_HOME_DELAY0);

            return;
        }
        default:
            OperationFailedException::fire(
                ErrorReport::SEND_ERROR_REPORT,
                "Failed to set date",
                env.console
            );
        }
    }
}























}
}
