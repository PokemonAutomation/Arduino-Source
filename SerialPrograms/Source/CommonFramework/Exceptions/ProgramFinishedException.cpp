/*  Program Finished Exception
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ProgramFinishedException.h"

namespace PokemonAutomation{


ProgramFinishedException::ProgramFinishedException(){}
ProgramFinishedException::ProgramFinishedException(std::string message)
    : m_message(message)
{}


ProgramFinishedException::ProgramFinishedException(
    std::string message,
    VideoStream& stream
)
    : m_message(message)
    , m_stream(&stream)
    , m_screenshot(stream.video().snapshot().frame)
{}
ProgramFinishedException::ProgramFinishedException(
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : m_message(message)
    , m_stream(stream)
    , m_screenshot(std::make_shared<ImageRGB32>(std::move(screenshot)))
{}
ProgramFinishedException::ProgramFinishedException(
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : m_message(message)
    , m_stream(stream)
    , m_screenshot(std::move(screenshot))
{}

void ProgramFinishedException::log(Logger& logger) const{
    logger.log(std::string(name()) + ": " + message(), COLOR_BLUE);
}

ImageViewRGB32 ProgramFinishedException::screenshot_view() const{
    if (m_screenshot){
        return *m_screenshot;
    }else{
        return ImageViewRGB32();
    }
}
std::shared_ptr<const ImageRGB32> ProgramFinishedException::screenshot() const{
    return m_screenshot;
}

VideoStream* ProgramFinishedException::video_stream() const{
    return m_stream;
}




}
