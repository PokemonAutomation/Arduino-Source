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
#include "CommonFramework/ErrorReports/ErrorReports.h"
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
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const std::string& label,
    const ImageViewRGB32& image
){
    report_error(
        &logger,
        program_info,
        label,
        {},
        image
    );
}
void dump_image(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    const std::string& label
){
    auto snapshot = console.video().snapshot();
    dump_image(console, program_info, label, snapshot);
}


void dump_image_and_throw_recoverable_exception(
    const ProgramInfo& program_info,
    ConsoleHandle& console,
    const std::string& error_name,
    const std::string& error_message,
    const ImageViewRGB32& screenshot
){
    OperationFailedException::fire(
        console, ErrorReport::SEND_ERROR_REPORT,
        error_message
    );
}













}
