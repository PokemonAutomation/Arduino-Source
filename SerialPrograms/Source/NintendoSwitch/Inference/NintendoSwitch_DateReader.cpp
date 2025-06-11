/*  Date Reader
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

//#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonTools/Images/ImageFilter.h"
#include "CommonTools/Async/InferenceRoutines.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "CommonTools/OCR/OCR_NumberReader.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "NintendoSwitch_DateReader.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch/Inference/NintendoSwitch_HomeMenuDetector.h"
#include "NintendoSwitch/Programs/DateSpam/NintendoSwitch_HomeToDateTime.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


using namespace std::chrono_literals;






DateChangeDetector_Switch1::DateChangeDetector_Switch1(Color color)
    : m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_top(0.50, 0.36, 0.45, 0.07)
    , m_window_text(0.05, 0.36, 0.10, 0.07)
    , m_jp_year(0.136, 0.61, 0.11, 0.09)
    , m_us_hour(0.473, 0.61, 0.06, 0.09)
    , m_jp_month_arrow(0.30, 0.50, 0.05, 0.06)
{}
void DateChangeDetector_Switch1::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_background_top);
    items.add(COLOR_RED, m_window_top);
    items.add(COLOR_RED, m_window_text);
    items.add(COLOR_RED, m_jp_year);
    items.add(COLOR_RED, m_us_hour);
    items.add(COLOR_RED, m_jp_month_arrow);
}
bool DateChangeDetector_Switch1::detect(const ImageViewRGB32& screen){
    ImageStats stats_background_top = image_stats(extract_box_reference(screen, m_background_top));
    if (stats_background_top.stddev.sum() > 10){
//        cout << "asdf" << endl;
        return false;
    }
    ImageStats stats_window_top = image_stats(extract_box_reference(screen, m_window_top));
//    cout << stats_window_top.average << stats_window_top.stddev << endl;
    if (stats_window_top.stddev.sum() > 10){
//        cout << "qwer" << endl;
        return false;
    }
    ImageStats stats_window_text = image_stats(extract_box_reference(screen, m_window_text));
//    cout << stats_window_text.stddev << endl;
    if (stats_window_text.stddev.sum() < 100){
//        cout << "zxcv" << endl;
        return false;
    }
//    cout << "stats_background_top: " << stats_background_top.average.sum() << endl;
//    cout << "stats_window_top: " << stats_window_top.average.sum() << endl;
    if (std::abs(stats_background_top.average.sum() - stats_window_top.average.sum()) < 50){
//        cout << "xcvb" << endl;
        return false;
    }

//    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 year_box = extract_box_reference(screen, m_jp_year);
    ImageStats year_stats = image_stats(year_box);
//    cout << year_stats.average << year_stats.stddev << endl;

    double stddev = year_stats.stddev.sum();
    if (stddev < 80){
//        cout << "sdfg" << endl;
        return false;
    }

    return true;
}
DateFormat DateChangeDetector_Switch1::detect_date_format(const ImageViewRGB32& screen) const{
    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hour);
    ImageStats stats_us_hours = image_stats(us_hours);

    if (stats_us_hours.stddev.sum() > 30){
        return DateFormat::US;
    }

    ImageViewRGB32 jp_arrow = extract_box_reference(screen, m_jp_month_arrow);
    ImageStats stats_arrow = image_stats(jp_arrow);
    if (stats_arrow.stddev.sum() > 30){
        return DateFormat::JP;
    }

    return DateFormat::EU;
}



DateChangeDetector_Switch2::DateChangeDetector_Switch2(Color color)
    : m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_bottom(0.50, 0.80, 0.45, 0.07)
    , m_window_text(0.05, 0.02, 0.10, 0.08)
    , m_jp_year(0.139, 0.436, 0.088, 0.095)
    , m_us_hour(0.473, 0.61, 0.06, 0.09)
    , m_jp_month_arrow(0.291705, 0.331675, 0.054986, 0.069652)
{}
void DateChangeDetector_Switch2::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_background_top);
    items.add(COLOR_RED, m_window_bottom);
    items.add(COLOR_RED, m_window_text);
    items.add(COLOR_RED, m_jp_year);
    items.add(COLOR_RED, m_us_hour);
    items.add(COLOR_RED, m_jp_month_arrow);
}
bool DateChangeDetector_Switch2::detect(const ImageViewRGB32& screen){
    ImageStats stats_background_top = image_stats(extract_box_reference(screen, m_background_top));
    if (stats_background_top.stddev.sum() > 10){
//        cout << "asdf" << endl;
        return false;
    }
    ImageStats stats_window_bottom = image_stats(extract_box_reference(screen, m_window_bottom));
//    cout << stats_window_bottom.average << stats_window_top.stddev << endl;
    if (stats_window_bottom.stddev.sum() > 10){
//        cout << "qwer" << endl;
        return false;
    }
    ImageStats stats_window_text = image_stats(extract_box_reference(screen, m_window_text));
//    cout << stats_window_text.stddev << endl;
    if (stats_window_text.stddev.sum() < 100){
//        cout << "zxcv" << endl;
        return false;
    }
//    cout << "stats_background_top: " << stats_background_top.average.sum() << endl;
//    cout << "stats_window_top: " << stats_window_top.average.sum() << endl;

    if (euclidean_distance(stats_background_top.average, stats_window_bottom.average) > 10){
//        cout << "xcvb" << endl;
        return false;
    }

//    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 year_box = extract_box_reference(screen, m_jp_year);
    ImageStats year_stats = image_stats(year_box);
//    cout << year_stats.average << year_stats.stddev << endl;

    double stddev = year_stats.stddev.sum();
    if (stddev < 80){
//        cout << "sdfg" << endl;
        return false;
    }

    return true;
}
DateFormat DateChangeDetector_Switch2::detect_date_format(const ImageViewRGB32& screen) const{
    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hour);
    ImageStats stats_us_hours = image_stats(us_hours);

    if (stats_us_hours.stddev.sum() > 30){
        return DateFormat::US;
    }

    ImageViewRGB32 jp_arrow = extract_box_reference(screen, m_jp_month_arrow);
    ImageStats stats_arrow = image_stats(jp_arrow);
    if (stats_arrow.stddev.sum() > 30){
        return DateFormat::JP;
    }

    return DateFormat::EU;
}












DateReader::DateReader(ConsoleHandle& console)
    : m_console(console)
    , m_switch1(COLOR_RED)
    , m_switch2(COLOR_MAGENTA)
    , m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_top(0.50, 0.36, 0.45, 0.07)
    , m_window_text(0.05, 0.36, 0.10, 0.07)
    , m_us_hour(0.473, 0.61, 0.06, 0.09)
    , m_jp_year(0.136, 0.61, 0.11, 0.09)
    , m_jp_month_arrow(0.30, 0.50, 0.05, 0.06)
    , m_switch1_US(COLOR_YELLOW)
    , m_switch1_EU(COLOR_CYAN)
    , m_switch1_JP(COLOR_PURPLE)
    , m_switch2_US(COLOR_YELLOW)
    , m_switch2_EU(COLOR_CYAN)
    , m_switch2_JP(COLOR_PURPLE)
{}
void DateReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_background_top);
    items.add(COLOR_RED, m_window_top);
    items.add(COLOR_RED, m_window_text);
    items.add(COLOR_RED, m_us_hour);
    items.add(COLOR_RED, m_jp_year);
    items.add(COLOR_RED, m_jp_month_arrow);
    m_switch1_US.make_overlays(items);
    m_switch1_EU.make_overlays(items);
    m_switch1_JP.make_overlays(items);
    m_switch2_US.make_overlays(items);
    m_switch2_EU.make_overlays(items);
    m_switch2_JP.make_overlays(items);
}
bool DateReader::detect(const ImageViewRGB32& screen){
    ConsoleType type = m_console.state().console_type();

    if (type == ConsoleType::Switch1){
        return m_switch1.detect(screen);
    }

    if (is_switch2(type)){
        return m_switch2.detect(screen);
    }

    throw UserSetupError(
        m_console,
        "Please select a valid Switch console type."
    );
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

    if (type == ConsoleType::Switch1){
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

    if (type == ConsoleType::Switch1){
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
