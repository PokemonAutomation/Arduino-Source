/*  Fatal Program Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "FatalProgramException.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


FatalProgramException::FatalProgramException(
    ErrorReport error_report,
    std::string message
)
    : m_error_report_mode(error_report)
    , m_message(message)
{}

FatalProgramException::FatalProgramException(
    ErrorReport error_report,
    std::string message,
    VideoStream& stream
)
    : m_error_report_mode(error_report)
    , m_message(message)
    , m_stream(&stream)
    , m_screenshot(stream.video().snapshot().frame)
{}

//  Construct exception with message with screenshot and (optionally) console information.
//  Use the provided screenshot instead of taking one with the console.
//  Store the console information (if provided) for stream history if requested later.
FatalProgramException::FatalProgramException(
    ErrorReport error_report,
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : m_error_report_mode(error_report)
    , m_message(message)
    , m_stream(stream)
    , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
{}

FatalProgramException::FatalProgramException(
    ErrorReport error_report,
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : m_error_report_mode(error_report)
    , m_message(message)
    , m_stream(stream)
    , m_screenshot(std::move(screenshot))
{}


void FatalProgramException::send_fatal_error_notif_and_telemetry_report(
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
