/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"
#include "ErrorDumper.h"

namespace PokemonAutomation{



QString dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const QString& label,
    const QImage& image
){
    static std::mutex lock;
    std::lock_guard<std::mutex> lg(lock);

    QDir().mkdir("ErrorDumps");
    QString name = "ErrorDumps/";
    name += QString::fromStdString(now_to_filestring());
    name += "-";
    name += label;
    name += ".png";
    logger.log("Saving failed inference image to: " + name, Qt::red);
    image.save(name);
    send_program_telemetry(
        logger, true, Qt::red,
        program_info,
        label,
        {},
        name
    );
    return name;
}


}
