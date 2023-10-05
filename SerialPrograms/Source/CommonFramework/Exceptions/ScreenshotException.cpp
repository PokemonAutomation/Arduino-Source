/*  Screenshot Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "ScreenshotException.h"

namespace PokemonAutomation{


ScreenshotException::ScreenshotException(ErrorReport error_report, std::string message)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
{}
ScreenshotException::ScreenshotException(ErrorReport error_report, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
    , m_screenshot(std::move(screenshot))
{}
ScreenshotException::ScreenshotException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot)
    : m_send_error_report(error_report)
    , m_message(std::move(message))
{
    if (take_screenshot){
        m_screenshot = console.video().snapshot().frame;
        if (m_screenshot == nullptr || !*m_screenshot){
            console.log("Camera returned empty screenshot. Is the camera frozen?", COLOR_RED);
        }
    }
}
void ScreenshotException::attach_screenshot(std::shared_ptr<const ImageRGB32> screenshot){
    m_screenshot = std::move(screenshot);
}
ImageViewRGB32 ScreenshotException::screenshot() const{
    if (m_screenshot){
        return *m_screenshot;
    }else{
        return ImageViewRGB32();
    }
}






}
