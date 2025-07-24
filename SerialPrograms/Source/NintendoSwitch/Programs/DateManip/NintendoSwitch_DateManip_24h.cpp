/*  Date Manipulation (24h)
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlayScopes.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "NintendoSwitch_DateManipTools.h"
#include "NintendoSwitch_DateManip_24h.h"

namespace PokemonAutomation{
namespace NintendoSwitch{



DateReader_24h::DateReader_24h(Color color)
    : m_color(color)
{}
void DateReader_24h::make_overlays(VideoOverlaySet& items) const{
    items.add(m_color, m_day);
    items.add(m_color, m_month);
    items.add(m_color, m_year);
    items.add(m_color, m_hour);
    items.add(m_color, m_minute);
}
DateTime DateReader_24h::read_date(Logger& logger, std::shared_ptr<const ImageRGB32> screen){
    logger.log("Attempting to read EU date...");

    DateTime date;

    date.day = (int8_t)DateReaderTools::read_box(logger, 1, 31, *screen, m_day);
    date.month = (int8_t)DateReaderTools::read_box(logger, 1, 12, *screen, m_month);
    date.year = (int16_t)DateReaderTools::read_box(logger, 2000, 2060, *screen, m_year);
    date.hour = (int8_t)DateReaderTools::read_box(logger, 0, 23, *screen, m_hour);
    date.minute = (int8_t)DateReaderTools::read_box(logger, 0, 59, *screen, m_minute);

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



DateReader_EU::DateReader_EU(Color color)
    : DateReader_24h(color)
{}
void DateReader_EU::set_date(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const DateTime& date
){
    stream.log("DateReader_EU::set_date()");

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
            move_horizontal(context, cursor_position, 5);
            return;
        }

        move_horizontal(context, cursor_position, 0);
        if (current.day < 0){
            stream.log("Failed to read day. Will not adjust.", COLOR_RED);
        }else{
            adjust_no_wrap(context, current.day, date.day);
        }

        move_horizontal(context, cursor_position, 1);
        if (current.month < 0){
            stream.log("Failed to read month. Will not adjust.", COLOR_RED);
        }else{
            adjust_wrap(context, 1, 12, current.month, date.month);
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
            adjust_wrap(context, 0, 23, current.hour, date.hour);
        }

        move_horizontal(context, cursor_position, 4);
        if (current.minute < 0){
            stream.log("Failed to read minutes. Will not adjust.", COLOR_RED);
        }else{
            adjust_wrap(context, 0, 59, current.minute, date.minute);
        }

        move_horizontal(context, cursor_position, 5);
    }

    throw_and_log<OperationFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set the hour after 10 attempts.",
        stream
    );
}


DateReader_Switch1_EU::DateReader_Switch1_EU(Color color)
    : DateReader_EU(color)
{
    m_day       = ImageFloatBox(0.145, 0.61, 0.06, 0.09);
    m_month     = ImageFloatBox(0.247, 0.61, 0.06, 0.09);
    m_year      = ImageFloatBox(0.355, 0.61, 0.11, 0.09);
    m_hour      = ImageFloatBox(0.528, 0.61, 0.06, 0.09);
    m_minute    = ImageFloatBox(0.629, 0.61, 0.06, 0.09);
}
DateReader_Switch2_EU::DateReader_Switch2_EU(Color color)
    : DateReader_EU(color)
{
    m_day       = ImageFloatBox(0.139, 0.436, 0.053, 0.095);
    m_month     = ImageFloatBox(0.246, 0.436, 0.053, 0.095);
    m_year      = ImageFloatBox(0.355, 0.436, 0.088, 0.095);
    m_hour      = ImageFloatBox(0.532, 0.436, 0.053, 0.095);
    m_minute    = ImageFloatBox(0.645, 0.436, 0.053, 0.095);
}



DateReader_JP::DateReader_JP(Color color)
    : DateReader_24h(color)
{}
void DateReader_JP::set_date(
    const ProgramInfo& info, VideoStream& stream, ProControllerContext& context,
    const DateTime& date
){
    stream.log("DateReader_JP::set_date()");

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
            move_horizontal(context, cursor_position, 5);
            return;
        }

        move_horizontal(context, cursor_position, 0);
        adjust_no_wrap(context, current.year, date.year);

        move_horizontal(context, cursor_position, 1);
        adjust_wrap(context, 1, 12, current.month, date.month);

        move_horizontal(context, cursor_position, 2);
        adjust_no_wrap(context, current.day, date.day);

        move_horizontal(context, cursor_position, 3);
        adjust_wrap(context, 0, 23, current.hour, date.hour);

        move_horizontal(context, cursor_position, 4);
        adjust_wrap(context, 0, 59, current.minute, date.minute);

        move_horizontal(context, cursor_position, 5);
    }

    throw_and_log<OperationFailedException>(
        stream.logger(), ErrorReport::SEND_ERROR_REPORT,
        "Failed to set the hour after 10 attempts.",
        stream
    );
}


DateReader_Switch1_JP::DateReader_Switch1_JP(Color color)
    : DateReader_JP(color)
{
    m_year      = ImageFloatBox(0.136, 0.61, 0.11, 0.09);
    m_month     = ImageFloatBox(0.295, 0.61, 0.06, 0.09);
    m_day       = ImageFloatBox(0.395, 0.61, 0.06, 0.09);
    m_hour      = ImageFloatBox(0.528, 0.61, 0.06, 0.09);
    m_minute    = ImageFloatBox(0.629, 0.61, 0.06, 0.09);
}
DateReader_Switch2_JP::DateReader_Switch2_JP(Color color)
    : DateReader_JP(color)
{
    m_year      = ImageFloatBox(0.139, 0.436, 0.088, 0.095);
    m_month     = ImageFloatBox(0.292, 0.436, 0.053, 0.095);
    m_day       = ImageFloatBox(0.410, 0.436, 0.053, 0.095);
    m_hour      = ImageFloatBox(0.532, 0.436, 0.053, 0.095);
    m_minute    = ImageFloatBox(0.645, 0.436, 0.053, 0.095);
}





}
}
