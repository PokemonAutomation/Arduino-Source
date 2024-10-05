/*  Unexpected Battle Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "CommonFramework/Tools/ConsoleHandle.h"
#include "UnexpectedBattleException.h"

namespace PokemonAutomation{


UnexpectedBattleException::UnexpectedBattleException(ErrorReport error_report, Logger& logger, std::string message)
    : ScreenshotException(error_report, std::move(message))
{
    logger.log(std::string(UnexpectedBattleException::name()) + ": " + m_message, COLOR_RED);
}
UnexpectedBattleException::UnexpectedBattleException(ErrorReport error_report, Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : ScreenshotException(error_report, std::move(message), std::move(screenshot))
{
    logger.log(std::string(UnexpectedBattleException::name()) + ": " + m_message, COLOR_RED);
}
UnexpectedBattleException::UnexpectedBattleException(ErrorReport error_report, ConsoleHandle& console, std::string message, bool take_screenshot)
    : ScreenshotException(error_report, console, std::move(message), take_screenshot)
{
    console.log(std::string(UnexpectedBattleException::name()) + ": " + m_message, COLOR_RED);
}


void UnexpectedBattleException::send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const{
    std::vector<std::pair<std::string, std::string>> embeds;
    if (!m_message.empty()){
        embeds.emplace_back(std::pair<std::string, std::string>("Message:", m_message));
    }
    if (m_send_error_report == ErrorReport::SEND_ERROR_REPORT && m_screenshot){
        std::string label = name();
        std::string filename = dump_image_alone(env.logger(), env.program_info(), label, *m_screenshot);
        send_program_telemetry(
            env.logger(), true, COLOR_RED,
            env.program_info(),
            label,
            embeds,
            filename
        );
    }
    send_program_notification(
        env, notification,
        COLOR_RED,
        "Program Error",
        std::move(embeds), "",
        screenshot()
    );
}





}
