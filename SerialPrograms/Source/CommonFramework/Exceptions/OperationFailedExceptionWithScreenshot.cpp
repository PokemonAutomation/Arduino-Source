/*  Operation Failed Exception with Screenshot
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/VideoStream.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "OperationFailedExceptionWithScreenshot.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{



OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    std::string message,
    VideoStream& stream
)
    : m_message(std::move(message))
    , m_stream(&stream)
    , m_screenshot(stream.video().snapshot().frame)
{
    if (m_screenshot == nullptr || !*m_screenshot){
        stream.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
    }
}
OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : m_message(std::move(message))
    , m_stream(stream)
    , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
{}
OperationFailedExceptionWithScreenshot::OperationFailedExceptionWithScreenshot(
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : m_message(std::move(message))
    , m_stream(stream)
    , m_screenshot(std::move(screenshot))
{}


// void ScreenshotException::add_stream_if_needed(VideoStream& stream){
//     if (m_stream == nullptr){
//         m_stream = &stream;
//     }
//     if (!m_screenshot){
//         m_screenshot = stream.video().snapshot();
//         if (m_screenshot == nullptr || !*m_screenshot){
//             stream.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
//         }
//     }
// }

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




}
