/*  Date Manipulation (US)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Qt/StringToolsQt.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "CommonTools/OCR/OCR_StringNormalization.h"
#include "NintendoSwitch/Commands/NintendoSwitch_Commands_Superscalar.h"
#include "NintendoSwitch_DateManipTools.h"
#include "NintendoSwitch_DateManip_US.h"

namespace PokemonAutomation{
namespace NintendoSwitch{




DateReader_US::DateReader_US(Color color)
    : m_color(color)
{}
void DateReader_US::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_month);
    items.add(m_color, m_day);
    items.add(m_color, m_year);
    items.add(m_color, m_hour);
    items.add(m_color, m_minute);
    items.add(m_color, m_ampm);
}
DateTime DateReader_US::read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen){
    logger.log("Attempting to read US date...");

    DateTime date;

    date.month = (int8_t)DateReaderTools::read_box(logger, 1, 12, *screen, m_month);
    date.day = (int8_t)DateReaderTools::read_box(logger, 1, 31, *screen, m_day);
    date.year = (int16_t)DateReaderTools::read_box(logger, 2000, 2060, *screen, m_year);
    date.minute = (int8_t)DateReaderTools::read_box(logger, 0, 59, *screen, m_minute);

    //  Hour
    {
        int hour = DateReaderTools::read_box(logger, 1, 12, *screen, m_hour);
        if (hour == 12){
            hour = 0;
        }

        ImageRGB32 us_ampm_filtered = DateReaderTools::filter_image(
            extract_box_reference(*screen, m_ampm)
        );

        std::string ampm_ocr = OCR::ocr_read(Language::English, us_ampm_filtered);
        if (ampm_ocr.back() == '\n'){
            ampm_ocr.pop_back();
        }
        std::string ampm = to_utf8(OCR::normalize_utf32(ampm_ocr));

        auto has_a = ampm.find('a') != std::string::npos;
        auto has_p = ampm.find('p') != std::string::npos;

        if (has_a && !has_p){
            //  Do nothing.
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> AM");
        }else if (!has_a && has_p){
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> PM");
            hour += 12;
        }else{
            logger.log("OCR Text: \"" + ampm_ocr + "\" -> \"" + ampm + "\" -> ??", COLOR_RED);
            hour = -1;
        }

        date.hour = (int8_t)hour;
    }

    if (date.day < 0 ||
        date.month < 0 ||
        date.year < 0 ||
        date.hour < 0 ||
        date.minute < 0
    ){
        report_error(
            &logger, ProgramInfo(),
            "Failed to read date.",
            {},
            *screen
        );
    }

    return date;
}
void DateReader_US::set_date(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const DateTime& date
){
    stream.log("DateReader_US::set_date()");

    int cursor_position = 0;

    for (size_t attempts = 0; attempts < 10; attempts++){
        context.wait_for_all_requests();
        context.wait_for(std::chrono::milliseconds(250));

        auto snapshot = stream.video().snapshot();
        DateTime current = read_date(stream.logger(), snapshot);
        if (current.year == date.year &&
            current.month == date.month &&
            current.day == date.day &&
            current.hour == date.hour &&
            current.minute == date.minute
        ){
            move_horizontal(context, cursor_position, 6);
            return;
        }

        move_horizontal(context, cursor_position, 0);
        if (current.month < 0){
            stream.log("Failed to read month. Will not adjust.", COLOR_RED);
        }else{
            adjust_wrap(context, 1, 12, current.month, date.month);
        }

        move_horizontal(context, cursor_position, 1);
        if (current.day < 0){
            stream.log("Failed to read day. Will not adjust.", COLOR_RED);
        }else{
            adjust_no_wrap(context, current.day, date.day);
        }

        move_horizontal(context, cursor_position, 2);
        if (current.year < 0){
            stream.log("Failed to read year. Will not adjust.", COLOR_RED);
        }else{
            adjust_no_wrap(context, current.year, date.year);
        }

        move_horizontal(context, cursor_position, 3);
        if (current.hour < 0){
            stream.log("Failed to read hour. Will not adjust.", COLOR_RED);
        }else{
            int8_t c = current.hour;
            int8_t t = date.hour;
            if (c >= 12) c -= 12;
            if (t >= 12) t -= 12;
            adjust_wrap(context, 0, 11, c, t);
        }

        move_horizontal(context, cursor_position, 4);
        if (current.minute < 0){
            stream.log("Failed to read minutes. Will not adjust.", COLOR_RED);
        }else{
            adjust_wrap(context, 0, 59, current.minute, date.minute);
        }

        move_horizontal(context, cursor_position, 5);
        if ((date.hour < 12) != (current.hour < 12)){
            ssf_issue_scroll_ptv(context, SSF_SCROLL_DOWN);
        }

        move_horizontal(context, cursor_position, 6);
    }

    throw_and_log<OperationFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set the hour after 10 attempts.",
        stream
    );
}




DateReader_Switch1_US::DateReader_Switch1_US(Color color)
    : DateReader_US(color)
{
    m_month     = ImageFloatBox(0.090, 0.61, 0.06, 0.09);
    m_day       = ImageFloatBox(0.193, 0.61, 0.06, 0.09);
    m_year      = ImageFloatBox(0.300, 0.61, 0.11, 0.09);
    m_hour      = ImageFloatBox(0.473, 0.61, 0.06, 0.09);
    m_minute    = ImageFloatBox(0.574, 0.61, 0.06, 0.09);
    m_ampm      = ImageFloatBox(0.663, 0.61, 0.07, 0.09);
}
DateReader_Switch2_US::DateReader_Switch2_US(Color color)
    : DateReader_US(color)
{
    m_month     = ImageFloatBox(0.080, 0.436, 0.053, 0.095);
    m_day       = ImageFloatBox(0.188, 0.436, 0.053, 0.095);
    m_year      = ImageFloatBox(0.298, 0.436, 0.088, 0.095);
    m_hour      = ImageFloatBox(0.463, 0.436, 0.053, 0.095);
    m_minute    = ImageFloatBox(0.575, 0.436, 0.053, 0.095);
    m_ampm      = ImageFloatBox(0.671, 0.436, 0.080, 0.095);
}






















}
}
