/*  Operation Failed Exception with Screenshot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "OperationFailedExceptionWithScreenshot.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    ErrorReportMode error_report_mode,
    std::string message,
    VideoStream& stream
)
    : OperationFailedException(error_report_mode, std::move(message))
    , m_stream(&stream)
    , m_screenshot(stream.video().snapshot().frame)
{
    if (m_screenshot == nullptr || !*m_screenshot){
        stream.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
    }
}
OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    ErrorReportMode error_report_mode,
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : OperationFailedException(error_report_mode, std::move(message))
    , m_stream(stream)
    , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
{}
OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    ErrorReportMode error_report_mode,
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : OperationFailedException(error_report_mode, std::move(message))
    , m_stream(stream)
    , m_screenshot(std::move(screenshot))
{}


[[noreturn]] void OperationFailedExceptionWithScreenshot::fire(
    ErrorReportMode error_report_mode,
    std::string message,
    VideoStream& stream
){
    throw_and_log<OperationFailedExceptionWithScreenshot>(
        stream.logger(),
        error_report_mode,
        std::move(message),
        stream
    );
}
[[noreturn]] void OperationFailedExceptionWithScreenshot::fire(
    ErrorReportMode error_report_mode,
    std::string message,
    VideoStream& stream,
    std::shared_ptr<const ImageRGB32> screenshot
){
    throw_and_log<OperationFailedExceptionWithScreenshot>(
        stream.logger(),
        error_report_mode,
        std::move(message),
        &stream,
        std::move(screenshot)
    );
}



ImageViewRGB32 OperationFailedExceptionWithScreenshot::screenshot_view() const{
    if (m_screenshot){
        return *m_screenshot;
    }else{
        return ImageViewRGB32();
    }
}
std::shared_ptr<const ImageRGB32> OperationFailedExceptionWithScreenshot::screenshot() const{
    return m_screenshot;
}

VideoStream* OperationFailedExceptionWithScreenshot::video_stream() const{
    return m_stream;
}

void OperationFailedExceptionWithScreenshot::send_recoverable_error_notif_and_telemetry_report(
    ProgramEnvironment& env,
    EventNotificationOption& notif_settings
){
    send_program_recoverable_error_notification_and_telemetry_report(
        env, &env.logger(), env.program_info(), 
        notif_settings, 
        error_report_mode(),
        message(),
        name(),
        *m_screenshot,
        &m_stream->history()
    );
}

void OperationFailedExceptionWithScreenshot::send_fatal_error_notif_and_telemetry_report(
    ProgramEnvironment& env,
    EventNotificationOption& notif_settings
){
    send_program_fatal_error_notification_and_telemetry_report(
        env, &env.logger(), env.program_info(),
        notif_settings,
        error_report_mode(),
        message(),
        name(),
        *m_screenshot,
        &m_stream->history()
    );
}






}
