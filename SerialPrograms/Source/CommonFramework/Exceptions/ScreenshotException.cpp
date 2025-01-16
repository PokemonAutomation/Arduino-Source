/*  Screenshot Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "ScreenshotException.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


ScreenshotException::ScreenshotException(ErrorReport error_report, std::string message)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
{}
ScreenshotException::ScreenshotException(
    ErrorReport error_report,
    std::string message,
    VideoStream& stream
)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
    , m_stream(&stream)
    , m_screenshot(stream.video().snapshot().frame)
{
    if (m_screenshot == nullptr || !*m_screenshot){
        stream.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
    }
}
ScreenshotException::ScreenshotException(
    ErrorReport error_report,
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
    , m_stream(stream)
    , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
{}
ScreenshotException::ScreenshotException(
    ErrorReport error_report,
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
    , m_stream(stream)
    , m_screenshot(std::move(screenshot))
{}


void ScreenshotException::add_stream_if_needed(VideoStream& stream){
    if (m_stream == nullptr){
        m_stream = &stream;
    }
    if (!m_screenshot){
        m_screenshot = stream.video().snapshot();
        if (m_screenshot == nullptr || !*m_screenshot){
            stream.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
        }
    }
}

ImageViewRGB32 ScreenshotException::screenshot() const{
    if (m_screenshot){
        return *m_screenshot;
    }else{
        return ImageViewRGB32();
    }
}


void ScreenshotException::send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const{
    std::vector<std::pair<std::string, std::string>> embeds;
    if (!m_message.empty()){
        embeds.emplace_back(std::pair<std::string, std::string>("Message:", m_message));
    }

    if (m_send_error_report == ErrorReport::SEND_ERROR_REPORT){
        report_error(
            &env.logger(),
            env.program_info(),
            name(),
            embeds,
            screenshot(),
            m_stream ? &m_stream->history() : nullptr
        );
    }

    send_program_notification(
        env, notification,
        color(),
        name(),
        std::move(embeds), "",
        screenshot()
    );
}





}
