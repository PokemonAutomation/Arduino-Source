/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/ErrorReports/ErrorReports.h"
//#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "OperationFailedException.h"

namespace PokemonAutomation{


OperationFailedException::OperationFailedException(ErrorReport error_report, Logger& logger, std::string message)
    : ScreenshotException(error_report, std::move(message))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
OperationFailedException::OperationFailedException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : ScreenshotException(error_report, std::move(message), std::move(screenshot))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
OperationFailedException::OperationFailedException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot)
    : ScreenshotException(error_report, console, std::move(message), take_screenshot)
{
    console.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}







}
