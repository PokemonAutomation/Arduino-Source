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
#include "OperationFailedException.h"

namespace PokemonAutomation{


OperationFailedException::OperationFailedException(Logger& logger, std::string message)
    : ScreenshotException(std::move(message))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
OperationFailedException::OperationFailedException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : ScreenshotException(std::move(message), std::move(screenshot))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
OperationFailedException::OperationFailedException(ConsoleHandle& console, std::string message, bool take_screenshot)
    : ScreenshotException(console, std::move(message), take_screenshot)
{
    console.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}


void OperationFailedException::send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const{
    std::vector<std::pair<std::string, std::string>> embeds;
    if (!m_message.empty()){
        embeds.emplace_back(std::pair<std::string, std::string>("Message:", m_message));
    }
    send_program_notification(
        env, notification,
        COLOR_RED,
        "Program Error",
        embeds, "",
        screenshot()
    );
    if (m_screenshot){
        std::string label = name();
        std::string filename = dump_image_alone(env.logger(), env.program_info(), label, *m_screenshot);
        send_program_telemetry(
            env.logger(), true, COLOR_RED,
            env.program_info(),
            label,
            std::move(embeds),
            filename
        );
    }
}





}
