/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/
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
    const ImageViewRGB32& image,
    const StreamHistorySession* stream_history
){
    report_error(
        &logger,
        program_info,
        label,
        {},
        image,
        stream_history
    );
}
void dump_image(
    Logger& logger,
    const ProgramInfo& program_info,
    VideoFeed& video,
    const std::string& label
){
    auto snapshot = video.snapshot();
    dump_image(logger, program_info, label, snapshot);
}


void dump_image_and_throw_recoverable_exception(
    const ProgramInfo& program_info,
    VideoStream& stream,
    const std::string& error_name,
    const std::string& error_message,
    const ImageViewRGB32& screenshot
){
    OperationFailedException::fire(
        ErrorReport::SEND_ERROR_REPORT,
        error_message,
        stream
    );
}













}
