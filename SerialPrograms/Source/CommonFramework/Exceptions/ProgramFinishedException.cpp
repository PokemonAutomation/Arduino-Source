/*  Program Finished Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "ProgramFinishedException.h"

namespace PokemonAutomation{


ProgramFinishedException::ProgramFinishedException(){}
ProgramFinishedException::ProgramFinishedException(Logger& logger, std::string message)
    : ScreenshotException(std::move(message))
{
    logger.log(std::string(ProgramFinishedException::name()) + ": " + m_message, COLOR_RED);
}
ProgramFinishedException::ProgramFinishedException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : ScreenshotException(std::move(message), std::move(screenshot))
{
    logger.log(std::string(ProgramFinishedException::name()) + ": " + m_message, COLOR_RED);
}
ProgramFinishedException::ProgramFinishedException(ConsoleHandle& console, std::string message, bool take_screenshot)
    : ScreenshotException(console, std::move(message), take_screenshot)
{
    console.log(std::string(ProgramFinishedException::name()) + ": " + m_message, COLOR_RED);
}


void ProgramFinishedException::send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const{
    std::vector<std::pair<std::string, std::string>> embeds;
    if (!m_message.empty()){
        embeds.emplace_back(std::pair<std::string, std::string>("Message:", m_message));
    }
    send_program_notification(
        env, notification,
        COLOR_GREEN,
        "Program Finished",
        std::move(embeds), "",
        screenshot()
    );
}





}
