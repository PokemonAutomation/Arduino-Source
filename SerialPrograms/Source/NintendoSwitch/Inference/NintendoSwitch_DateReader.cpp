/*  Date Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
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
#include "NintendoSwitch/Inference/NintendoSwitch_DetectHome.h"
#include "NintendoSwitch/Programs/NintendoSwitch_Navigation.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


using namespace std::chrono_literals;


DateReader::DateReader()
    : m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_top(0.50, 0.36, 0.45, 0.07)
    , m_window_text(0.05, 0.36, 0.10, 0.07)
    , m_jp_month_arrow(0.30, 0.50, 0.05, 0.06)
    , m_us_month(0.090, 0.61, 0.06, 0.09)
    , m_us_day(0.193, 0.61, 0.06, 0.09)
    , m_us_year(0.300, 0.61, 0.11, 0.09)
    , m_us_hour(0.473, 0.61, 0.06, 0.09)
    , m_us_minute(0.574, 0.61, 0.06, 0.09)
    , m_us_ampm(0.663, 0.61, 0.07, 0.09)
    , m_eu_day(0.145, 0.61, 0.06, 0.09)
    , m_eu_month(0.247, 0.61, 0.06, 0.09)
    , m_eu_year(0.355, 0.61, 0.11, 0.09)
    , m_24_hour(0.528, 0.61, 0.06, 0.09)
    , m_24_minute(0.629, 0.61, 0.06, 0.09)
    , m_jp_year(0.136, 0.61, 0.11, 0.09)
    , m_jp_month(0.295, 0.61, 0.06, 0.09)
    , m_jp_day(0.395, 0.61, 0.06, 0.09)
{}
void DateReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_background_top);
    items.add(COLOR_RED, m_window_top);
    items.add(COLOR_RED, m_window_text);
    items.add(COLOR_RED, m_jp_month_arrow);
    items.add(COLOR_YELLOW, m_us_month);
    items.add(COLOR_YELLOW, m_us_day);
    items.add(COLOR_YELLOW, m_us_year);
    items.add(COLOR_YELLOW, m_us_hour);
    items.add(COLOR_YELLOW, m_us_minute);
    items.add(COLOR_YELLOW, m_us_ampm);
    items.add(COLOR_CYAN, m_eu_day);
    items.add(COLOR_CYAN, m_eu_month);
    items.add(COLOR_CYAN, m_eu_year);
    items.add(COLOR_GREEN, m_24_hour);
    items.add(COLOR_GREEN, m_24_minute);
    items.add(COLOR_PURPLE, m_jp_year);
    items.add(COLOR_PURPLE, m_jp_month);
    items.add(COLOR_PURPLE, m_jp_day);
}
bool DateReader::detect(const ImageViewRGB32& screen) const{
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

    ImageViewRGB32 us_hours = extract_box_reference(screen, m_jp_year);
    ImageStats stats_us_hours = image_stats(us_hours);
//    cout << stats_us_hours.average << stats_us_hours.stddev << endl;

    double stddev = stats_us_hours.stddev.sum();
    if (stddev < 80){
//        cout << "sdfg" << endl;
        return false;
    }

    return true;
}


void DateReader::set_hours(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    uint8_t hour
) const{
    context.wait_for_all_requests();
    {
        auto snapshot = stream.video().snapshot();
        if (!detect(snapshot)){
            throw_and_log<FatalProgramException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Expected date change menu.",
                stream
            );
        }
    }

    for (size_t attempts = 0; attempts < 10; attempts++){
        //  Move cursor out of the way.
        for (size_t c = 0; c < 7; c++){
            ssf_issue_scroll_ptv(context, DPAD_LEFT);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(250));

        //  Read the hour.
        VideoSnapshot snapshot = stream.video().snapshot();
//        int8_t current_hour = read_hours(stream, snapshot);
        int8_t current_hour = read_date(stream.logger(), snapshot).second.hour;

        if (current_hour < 0){
            throw_and_log<FatalProgramException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Unable to read the hour.",
                stream
            );
        }

        //  We're done.
        if (current_hour == hour){
            for (size_t c = 0; c < 7; c++){
                ssf_issue_scroll_ptv(context, DPAD_RIGHT);
            }
            return;
        }

        //  Move the cursor to the hour.
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);
        ssf_issue_scroll_ptv(context, DPAD_RIGHT);

        ImageViewRGB32 us_hours = extract_box_reference(snapshot, m_us_hour);
        ImageStats stats_us_hours = image_stats(us_hours);
        double stddev = stats_us_hours.stddev.sum();
        bool format_us = stddev > 30;
        if (format_us){
            uint8_t diff = (24 + (uint8_t)hour - (uint8_t)current_hour) % 12;
            if (diff < 6){
                for (size_t c = 0; c < diff; c++){
                    ssf_issue_scroll_ptv(context, DPAD_UP);
                }
            }else{
                for (size_t c = diff; c < 12; c++){
                    ssf_issue_scroll_ptv(context, DPAD_DOWN);
                }
            }
            if ((hour < 12) != (current_hour < 12)){
                ssf_issue_scroll_ptv(context, DPAD_RIGHT);
                ssf_issue_scroll_ptv(context, DPAD_RIGHT);
                ssf_issue_scroll_ptv(context, DPAD_DOWN);
            }
        }else{
            uint8_t diff = (24 + (uint8_t)hour - (uint8_t)current_hour) % 24;
            if (diff < 12){
                for (size_t c = 0; c < diff; c++){
                    ssf_issue_scroll_ptv(context, DPAD_UP);
                }
            }else{
                for (size_t c = diff; c < 24; c++){
                    ssf_issue_scroll_ptv(context, DPAD_DOWN);
                }
            }
        }
    }

//    auto snapshot = console.video().snapshot();
    throw_and_log<FatalProgramException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set the hour after 10 attempts.",
        stream
    );
}


std::pair<DateFormat, DateTime> DateReader::read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen) const{
    if (!detect(*screen)){
        throw_and_log<OperationFailedException>(
            logger, ErrorReport::SEND_ERROR_REPORT,
            "Not on date change screen.",
            nullptr,
            std::move(screen)
        );
    }

    ImageStats stats_window_top = image_stats(extract_box_reference(*screen, m_window_top));
//    cout << stats_window_top.average << stats_window_top.stddev << endl;
    if (stats_window_top.stddev.sum() > 10){
        throw_and_log<OperationFailedException>(
            logger, ErrorReport::SEND_ERROR_REPORT,
            "Not on date change screen.",
            nullptr,
            std::move(screen)
        );
    }

    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 us_hours = extract_box_reference(*screen, m_us_hour);
    ImageStats stats_us_hours = image_stats(us_hours);

    if (stats_us_hours.stddev.sum() > 30){
        return {DateFormat::US, read_date_us(logger, std::move(screen), white_theme)};
    }

    ImageViewRGB32 jp_arrow = extract_box_reference(*screen, m_jp_month_arrow);
    ImageStats stats_arrow = image_stats(jp_arrow);
    if (stats_arrow.stddev.sum() > 30){
        return {DateFormat::JP, read_date_jp(logger, std::move(screen), white_theme)};
    }

    return {DateFormat::EU, read_date_eu(logger, std::move(screen), white_theme)};
}
ImageRGB32 DateReader::filter_image(const ImageViewRGB32& image, bool white_theme){
    ImageRGB32 filtered = to_blackwhite_rgb32_range(
        image,
        0xff000000, white_theme ? 0xffff7fff : 0xff7f7f7f,
        white_theme
    );
    return filtered;
}
int DateReader::read_box(
    Logger& logger,
    int min, int max,
    const ImageViewRGB32& screen, const ImageFloatBox& box,
    bool white_theme
) const{
    ImageViewRGB32 cropped = extract_box_reference(screen, box);

    int value;
    if (white_theme){
        value = OCR::read_number_waterfill(
            logger, cropped,
            0xff000000, 0xffff7fff, true
        );
    }else{
        value = OCR::read_number_waterfill(
            logger, cropped,
            0xff000000, 0xff7f7f7f, false
        );
    }

    if (value < min || value > max){
        value = -1;
    }
    return value;
}
DateTime DateReader::read_date_us(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const{
    logger.log("Attempting to read US date...");

    DateTime date;

    date.month = (int8_t)read_box(logger, 1, 12, *screen, m_us_month, white_theme);
    date.day = (int8_t)read_box(logger, 1, 31, *screen, m_us_day, white_theme);
    date.year = (int16_t)read_box(logger, 2000, 2060, *screen, m_us_year, white_theme);
    date.minute = (int8_t)read_box(logger, 0, 59, *screen, m_us_minute, white_theme);

    //  Hour
    {
        int hour = read_box(logger, 1, 12, *screen, m_us_hour, white_theme);
        if (hour == 12){
            hour = 0;
        }

        ImageRGB32 us_ampm_filtered = filter_image(
            extract_box_reference(*screen, m_us_ampm),
            white_theme
        );

        std::string ampm_ocr = OCR::ocr_read(Language::English, us_ampm_filtered);
        if (ampm_ocr.back() == '\n'){
            ampm_ocr.pop_back();
        }
        std::string ampm = to_utf8(OCR::normalize_utf32(ampm_ocr));

        if (ampm == "am"){
            //  Do nothing.
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> AM");
        }else if (ampm == "pm"){
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> PM");
            hour += 12;
        }else{
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> ??", COLOR_RED);
            hour = -1;
        }

        date.hour = (int8_t)hour;
    }

    return date;
}
DateTime DateReader::read_date_eu(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const{
    logger.log("Attempting to read EU date...");

    DateTime date;

    date.day = (int8_t)read_box(logger, 1, 31, *screen, m_eu_day, white_theme);
    date.month = (int8_t)read_box(logger, 1, 12, *screen, m_eu_month, white_theme);
    date.year = (int16_t)read_box(logger, 2000, 2060, *screen, m_eu_year, white_theme);
    date.hour = (int8_t)read_box(logger, 0, 23, *screen, m_24_hour, white_theme);
    date.minute = (int8_t)read_box(logger, 0, 59, *screen, m_24_minute, white_theme);

    return date;
}
DateTime DateReader::read_date_jp(Logger& logger, std::shared_ptr<const ImageRGB32> screen, bool white_theme) const{
    logger.log("Attempting to read JP date...");

    DateTime date;

    date.day = (int8_t)read_box(logger, 1, 31, *screen, m_jp_day, white_theme);
    date.month = (int8_t)read_box(logger, 1, 12, *screen, m_jp_month, white_theme);
    date.year = (int16_t)read_box(logger, 2000, 2060, *screen, m_jp_year, white_theme);
    date.hour = (int8_t)read_box(logger, 0, 23, *screen, m_24_hour, white_theme);
    date.minute = (int8_t)read_box(logger, 0, 59, *screen, m_24_minute, white_theme);

    return date;
}


void DateReader::move_cursor(ProControllerContext& context, int current, int desired){
    while (current < desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        current++;
    }
    while (current > desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        current--;
    }
}
void DateReader::adjust_year(ProControllerContext& context, int current, int desired){
    while (current < desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
        current++;
    }
    while (current > desired){
        ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        current--;
    }
}
void DateReader::adjust_month(ProControllerContext& context, int current, int desired){
    int diff = desired - current;
    if ((diff >= 0 && diff <= 6) || (diff < 0 && diff < -6)){
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
            current++;
            if (current > 12){
                current -= 12;
            }
        }
    }else{
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
            current--;
            if (current < 1){
                current += 12;
            }
        }
    }
}
void DateReader::adjust_hour_24(ProControllerContext& context, int current, int desired){
    int diff = desired - current;
    if ((diff >= 0 && diff <= 12) || (diff < 0 && diff < -12)){
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
            current++;
            if (current > 23){
                current -= 24;
            }
        }
    }else{
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
            current--;
            if (current < 0){
                current += 24;
            }
        }
    }
}
void DateReader::adjust_minute(ProControllerContext& context, int current, int desired){
    int diff = desired - current;
    if ((diff >= 0 && diff <= 30) || (diff < 0 && diff < -30)){
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
            current++;
            if (current > 59){
                current -= 60;
            }
        }
    }else{
        while (current != desired){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
            current--;
            if (current < 0){
                current += 60;
            }
        }
    }
}

void DateReader::set_date(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const DateTime& date
) const{
    context.wait_for_all_requests();

    {
        auto snapshot = stream.video().snapshot();
        if (!detect(snapshot)){
            throw_and_log<OperationFailedException>(
                stream.logger(), ErrorReport::SEND_ERROR_REPORT,
                "Expected date change menu.",
                &stream,
                snapshot
            );
        }
    }

    bool cursor_on_right = false;

    for (size_t attempts = 0; attempts < 10; attempts++){
        ssf_flush_pipeline(context);
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(250));

        auto snapshot = stream.video().snapshot();
        std::pair<DateFormat, DateTime> current = read_date(stream.logger(), snapshot);
        if (current.second.year == date.year &&
            current.second.month == date.month &&
            current.second.day == date.day &&
            current.second.hour == date.hour &&
            current.second.minute == date.minute
        ){
            if (!cursor_on_right){
                for (size_t c = 0; c < 7; c++){
                    ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
                }
            }
            return;
        }

        //  Move cursor to the left.
        for (size_t c = 0; c < 7; c++){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_LEFT);
        }

        switch (current.first){
        case DateFormat::US:{
            adjust_month(context, current.second.month, date.month);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_year(context, current.second.day, date.day);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_year(context, current.second.year, date.year);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

            uint8_t diff = (24 + date.hour - current.second.hour) % 12;
            if (diff < 6){
                for (size_t c = 0; c < diff; c++){
                    ssf_issue_scroll_ptv(context, SSF_SCROLL_UP);
                }
            }else{
                for (size_t c = diff; c < 12; c++){
                    ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
                }
            }

            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_minute(context, current.second.minute, date.minute);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            if ((date.hour < 12) != (current.second.hour < 12)){
                ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
            }
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);

            break;
        }
        case DateFormat::EU:
            adjust_year(context, current.second.day, date.day);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_month(context, current.second.month, date.month);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_year(context, current.second.year, date.year);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_hour_24(context, current.second.hour, date.hour);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_minute(context, current.second.minute, date.minute);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            break;
        case DateFormat::JP:
            adjust_year(context, current.second.year, date.year);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_month(context, current.second.month, date.month);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_year(context, current.second.day, date.day);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_hour_24(context, current.second.hour, date.hour);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            adjust_minute(context, current.second.minute, date.minute);
            ssf_issue_scroll_ptv(context, SSF_SCROLL_RIGHT);
            break;
        }

        cursor_on_right = true;
    }

    ssf_flush_pipeline(context);
    throw_and_log<FatalProgramException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set the hour after 10 attempts.",
        stream
    );
}

void change_date(
    SingleSwitchProgramEnvironment& env, ProControllerContext& context,
    const DateTime& date
){
    Milliseconds timing_variation = context->timing_variation();
    while (true){
        context.wait_for_all_requests();

        HomeWatcher home;
        DateChangeWatcher date_reader;
        int ret = wait_until(
            env.console, context, std::chrono::seconds(10),
            {
                home,
                date_reader
            }
        );
        switch (ret){
        case 0:
            home_to_date_time(context, true, false);
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
