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
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message))
{}


ProgramFinishedException::ProgramFinishedException(
    std::string message,
    VideoStream& stream
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), stream)
{}
ProgramFinishedException::ProgramFinishedException(
    ErrorReport error_report,
    std::string message,
    VideoStream* stream,
    ImageRGB32 screenshot
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), stream, std::move(screenshot))
{}
ProgramFinishedException::ProgramFinishedException(
    std::string message,
    VideoStream* stream,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), stream, std::move(screenshot))
{}

void ProgramFinishedException::log(Logger& logger) const{
    logger.log(std::string(name()) + ": " + message(), COLOR_BLUE);
}




}
