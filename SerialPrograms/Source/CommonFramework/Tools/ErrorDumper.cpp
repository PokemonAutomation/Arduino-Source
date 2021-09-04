/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/PrettyPrint.h"
#include "ErrorDumper.h"

namespace PokemonAutomation{


QString dump_image(Logger& logger, const QImage& image, const QString& label){
    QDir().mkdir("ErrorDumps");
    QString name = "ErrorDumps/";
    name += QString::fromStdString(now_to_filestring());
    name += "-";
    name += label;
    name += ".png";
    logger.log("Saving failed inference image to: " + name, Qt::red);
    image.save(name);
    return std::move(name);
}


}
