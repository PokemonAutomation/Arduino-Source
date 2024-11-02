/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "OliveActionFailedException.h"

namespace PokemonAutomation{


OliveActionFailedException::OliveActionFailedException(ErrorReport error_report, Logger& logger, std::string message, OliveFail fail_reason)
    : OperationFailedException(error_report, logger, std::move(message))
    , m_fail_reason(fail_reason)
{}
OliveActionFailedException::OliveActionFailedException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot, OliveFail fail_reason)
    : OperationFailedException(error_report, logger, std::move(message), std::move(screenshot))
    , m_fail_reason(fail_reason)
{}
OliveActionFailedException::OliveActionFailedException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot, OliveFail fail_reason)
    : OperationFailedException(error_report, console, std::move(message), take_screenshot)
    , m_fail_reason(fail_reason)
{}





}
