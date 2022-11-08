/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <QDir>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "ConsoleHandle.h"
#include "ErrorDumper.h"
#include "ProgramEnvironment.h"
namespace PokemonAutomation{



std::string dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
){
    static std::mutex lock;
    std::lock_guard<std::mutex> lg(lock);

    QDir().mkdir("ErrorDumps");
    std::string name = "ErrorDumps/";
    name += now_to_filestring();
    name += "-";
    name += label;
    name += ".png";
    logger.log("Saving failed inference image to: " + name, COLOR_RED);
    image.save(name);
    send_program_telemetry(
        logger, true, COLOR_RED,
        program_info,
        label,
        {},
        name
    );
    return name;
}

void dump_image_and_throw_recoverable_exception(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    EventNotificationOption& notification_error,
    const std::string& error_name,
    const std::string& message
){
    // m_stats.m_errors++;
    env.log(message, COLOR_RED);
    console.overlay().add_log("Error: " + error_name, COLOR_RED);
    std::shared_ptr<const ImageRGB32> screen = console.video().snapshot();
    dump_image(
        console, env.program_info(),
        error_name,
        *screen
    );
    send_program_recoverable_error_notification(
        env,
        notification_error,
        message,
        *screen
    );
    throw OperationFailedException(console, message);
}


}
