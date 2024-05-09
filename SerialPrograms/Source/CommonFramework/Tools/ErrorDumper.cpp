/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Exceptions/OperationFailedException.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
//#include "CommonFramework/Notifications/EventNotificationOption.h"
#include "CommonFramework/Notifications/ProgramInfo.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "CommonFramework/VideoPipeline/VideoFeed.h"
//#include "CommonFramework/VideoPipeline/VideoOverlay.h"
#include "ConsoleHandle.h"
#include "ErrorDumper.h"
//#include "ProgramEnvironment.h"
namespace PokemonAutomation{



std::string dump_image_alone(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
){
    static std::mutex lock;
    std::lock_guard<std::mutex> lg(lock);

    QDir().mkdir(ERROR_PATH().c_str());
    std::string name = ERROR_PATH() + now_to_filestring();
    name += "-";
    name += label;
    name += ".png";
    logger.log("Saving failed inference image to: " + name, COLOR_RED);
    image.save(name);
    return name;
}
std::string dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
){
    std::string name = dump_image_alone(logger, program_info, label, image);
    send_program_telemetry(
        logger, true, COLOR_RED,
        program_info,
        label,
        {},
        name
    );
    return name;
}
std::string dump_image(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    const std::string& label
){
    auto snapshot = console.video().snapshot();
    return dump_image(console, program_info, label, snapshot);
}

#if 0
void dump_image_and_throw_recoverable_exception(
    ProgramEnvironment& env,
    ConsoleHandle& console,
    EventNotificationOption& notification_error,
    const std::string& error_name,
    const std::string& message
){
#if 0
    // m_stats.m_errors++;
    console.overlay().add_log("Error: " + error_name, COLOR_RED);
    VideoSnapshot screen = console.video().snapshot();
    dump_image(
        console, env.program_info(),
        error_name,
        screen
    );
    send_program_recoverable_error_notification(
        env,
        notification_error,
        message,
        screen
    );
    throw OperationFailedException(console, message);
#else
    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT, console,
        message,
        true
    );
#endif
}
#endif

void dump_image_and_throw_recoverable_exception(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    const std::string& error_name,
    const std::string& error_message,
    const ImageViewRGB32& screenshot
){
#if 0
    console.overlay().add_log("Error: " + error_name, COLOR_RED);
    if (screenshot){
        dump_image(
            console, program_info,
            error_name,
            screenshot
        );
    }else{
        VideoSnapshot screen = console.video().snapshot();
        dump_image(
            console, program_info,
            error_name,
            screen
        );
    }
    throw OperationFailedException(console, error_message);
#else
    throw OperationFailedException(
        ErrorReport::SEND_ERROR_REPORT, console,
        error_message,
        true
    );
#endif
}













}
