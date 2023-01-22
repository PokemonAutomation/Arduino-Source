/*  Operation Failed Exception
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <memory>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "CommonFramework/ImageTypes/ImageRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/Tools/ErrorDumper.h"
#include "CommonFramework/Tools/ProgramEnvironment.h"
#include "OperationFailedException.h"

namespace PokemonAutomation{



OperationFailedException::~OperationFailedException() = default;
OperationFailedException::OperationFailedException(OperationFailedException&&) = default;
OperationFailedException& OperationFailedException::operator=(OperationFailedException&&) = default;

OperationFailedException::OperationFailedException(Logger& logger, std::string message)
    : m_message(std::move(message))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
OperationFailedException::OperationFailedException(Logger& logger, std::string message, std::shared_ptr<const ImageRGB32> screenshot)
    : m_message(std::move(message))
    , m_screenshot(std::move(screenshot))
{
    logger.log(std::string(OperationFailedException::name()) + ": " + m_message, COLOR_RED);
}
ImageViewRGB32 OperationFailedException::screenshot() const{
    if (m_screenshot){
        return *m_screenshot;
    }else{
        return ImageViewRGB32();
    }
}


void OperationFailedException::send_notification(ProgramEnvironment& env, EventNotificationOption& notification) const{
    send_program_notification(
        env, notification,
        COLOR_RED,
        "Program Error",
        {{"Message:", m_message}}, "",
        screenshot()
    );
    if (m_screenshot){
        std::string label = name();
        std::string filename = dump_image_alone(env.logger(), env.program_info(), label, *m_screenshot);
        send_program_telemetry(
            env.logger(), true, COLOR_RED,
            env.program_info(),
            label,
            {{"Message", m_message}},
            filename
        );
    }
}





}
