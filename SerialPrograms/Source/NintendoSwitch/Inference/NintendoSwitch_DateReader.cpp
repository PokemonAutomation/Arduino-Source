/*  Date Reader
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Exceptions/FatalProgramException.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ImageTools/ImageStats.h"
#include "CommonFramework/ImageTools/ImageFilter.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "CommonFramework/OCR/OCR_NumberReader.h"
#include "CommonFramework/OCR/OCR_StringNormalization.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_PushButtons.h"
#include "NintendoSwitch_DateReader.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace NintendoSwitch{


DateReader::DateReader()
    : m_background_top(0.50, 0.02, 0.45, 0.08)
    , m_window_top(0.50, 0.36, 0.45, 0.07)
    , m_window_text(0.05, 0.36, 0.10, 0.07)
    , m_us_hours(0.470, 0.60, 0.06, 0.10)
    , m_us_min(0.572, 0.60, 0.06, 0.10)
    , m_us_ampm(0.665, 0.60, 0.06, 0.10)
    , m_24_hours(0.525, 0.60, 0.06, 0.10)
    , m_24_min(0.626, 0.60, 0.06, 0.10)
{}
void DateReader::make_overlays(VideoOverlaySet& items) const{
    items.add(COLOR_RED, m_background_top);
    items.add(COLOR_RED, m_window_top);
    items.add(COLOR_RED, m_window_text);
    items.add(COLOR_YELLOW, m_us_hours);
    items.add(COLOR_YELLOW, m_us_min);
    items.add(COLOR_YELLOW, m_us_ampm);
    items.add(COLOR_CYAN, m_24_hours);
    items.add(COLOR_CYAN, m_24_min);
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
    if (stats_background_top.average.sum() > stats_window_top.average.sum()){
        return false;
    }

//    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hours);
    ImageStats stats_us_hours = image_stats(us_hours);
//    cout << stats_us_hours.average << stats_us_hours.stddev << endl;

    double stddev = stats_us_hours.stddev.sum();
    if (10 < stddev && stddev < 80){
        return false;
    }

    return true;
}
int8_t DateReader::read_hours(Logger& logger, const ImageViewRGB32& screen) const{
    if (!detect(screen)){
        return -1;
    }

    ImageStats stats_window_top = image_stats(extract_box_reference(screen, m_window_top));
//    cout << stats_window_top.average << stats_window_top.stddev << endl;
    if (stats_window_top.stddev.sum() > 10){
        return false;
    }

    bool white_theme = stats_window_top.average.sum() > 600;

    ImageViewRGB32 us_hours = extract_box_reference(screen, m_us_hours);
    ImageStats stats_us_hours = image_stats(us_hours);

    double stddev = stats_us_hours.stddev.sum();
    bool format_us = stddev > 30;
    if (format_us){
        ImageRGB32 us_hours_filtered = to_blackwhite_rgb32_range(us_hours, 0xff000000, 0xff7f7f7f, white_theme);
        int hours = OCR::read_number(logger, us_hours_filtered);
        if (hours < 1 || hours > 12){
            return -1;
        }
        if (hours == 12){
            hours = 0;
        }

        ImageViewRGB32 us_ampm = extract_box_reference(screen, m_us_ampm);
        ImageRGB32 us_ampm_filtered = to_blackwhite_rgb32_range(us_ampm, 0xff000000, 0xff7f7f7f, white_theme);

        std::string ampm = to_utf8(OCR::normalize_utf32(OCR::ocr_read(Language::English, us_ampm_filtered)));
        if (ampm == "am"){
            //  Do nothing.
        }else if (ampm == "pm"){
            hours += 12;
        }else{
            return -1;
        }
        return (int8_t)hours;
    }else{
        ImageViewRGB32 h24_hours = extract_box_reference(screen, m_24_hours);
        ImageRGB32 h24_hours_filtered = to_blackwhite_rgb32_range(h24_hours, 0xff000000, 0xff7f7f7f, white_theme);
        int hours = OCR::read_number(logger, h24_hours_filtered);
        if (hours < 0 || hours > 23){
            return -1;
        }
        return (int8_t)hours;
    }
}


void DateReader::set_hours(
    const ProgramInfo& info, ConsoleHandle& console, BotBaseContext& context,
    uint8_t hour
){
    context.wait_for_all_requests();
    {
        auto snapshot = console.video().snapshot();
        if (!detect(snapshot)){
            throw FatalProgramException(console, "Expected date change menu.", true);
        }
    }

    for (size_t attempts = 0; attempts < 10; attempts++){
        //  Move cursor out of the way.
        for (size_t c = 0; c < 7; c++){
            pbf_press_dpad(context, DPAD_LEFT, 5, 3);
        }
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(250));

        //  Read the hour.
        VideoSnapshot snapshot = console.video().snapshot();
        int8_t current_hour = read_hours(console.logger(), snapshot);
        if (current_hour < 0){
            throw FatalProgramException(console, "Unable to read the hour.", true);
        }

        //  We're done.
        if (current_hour == hour){
            for (size_t c = 0; c < 7; c++){
                pbf_press_dpad(context, DPAD_RIGHT, 5, 3);
            }
            return;
        }

        //  Move the cursor to the hour.
        pbf_press_dpad(context, DPAD_RIGHT, 5, 3);
        pbf_press_dpad(context, DPAD_RIGHT, 5, 3);
        pbf_press_dpad(context, DPAD_RIGHT, 5, 3);

        ImageViewRGB32 us_hours = extract_box_reference(snapshot, m_us_hours);
        ImageStats stats_us_hours = image_stats(us_hours);
        double stddev = stats_us_hours.stddev.sum();
        bool format_us = stddev > 30;
        if (format_us){
            uint8_t diff = (24 + (uint8_t)hour - (uint8_t)current_hour) % 12;
            if (diff < 6){
                for (size_t c = 0; c < diff; c++){
                    pbf_press_dpad(context, DPAD_UP, 5, 3);
                }
            }else{
                for (size_t c = diff; c < 12; c++){
                    pbf_press_dpad(context, DPAD_DOWN, 5, 3);
                }
            }
            if ((hour < 12) != (current_hour < 12)){
                pbf_press_dpad(context, DPAD_RIGHT, 5, 3);
                pbf_press_dpad(context, DPAD_RIGHT, 5, 3);
                pbf_press_dpad(context, DPAD_DOWN, 5, 3);
            }
        }else{
            uint8_t diff = (24 + (uint8_t)hour - (uint8_t)current_hour) % 24;
            if (diff < 12){
                for (size_t c = 0; c < diff; c++){
                    pbf_press_dpad(context, DPAD_UP, 5, 3);
                }
            }else{
                for (size_t c = diff; c < 24; c++){
                    pbf_press_dpad(context, DPAD_DOWN, 5, 3);
                }
            }
        }
    }

    auto snapshot = console.video().snapshot();
    throw FatalProgramException(console, "Failed to set the hour after 10 attempts.", true);
}







}
}
