/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <mutex>
#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/ImageTypes/ImageViewRGB32.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ErrorDumper.h"

namespace PokemonAutomation{



std::string dump_image(
    LoggerQt& logger,
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


}
