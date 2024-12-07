/*  Program Finished Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "ProgramFinishedException.h"

namespace PokemonAutomation{


ProgramFinishedException::ProgramFinishedException(){}
ProgramFinishedException::ProgramFinishedException(std::string message)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message))
{}


ProgramFinishedException::ProgramFinishedException(
    std::string message,
    ConsoleHandle& console
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), console)
{}
ProgramFinishedException::ProgramFinishedException(
    ErrorReport error_report,
    std::string message,
    ConsoleHandle* console,
    ImageRGB32 screenshot
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), console, std::move(screenshot))
{}
ProgramFinishedException::ProgramFinishedException(
    std::string message,
    ConsoleHandle* console,
    std::shared_ptr<const ImageRGB32> screenshot
)
    : ScreenshotException(ErrorReport::NO_ERROR_REPORT, std::move(message), console, std::move(screenshot))
{}

void ProgramFinishedException::log(Logger& logger) const{
    logger.log(std::string(name()) + ": " + message(), COLOR_BLUE);
}




}
