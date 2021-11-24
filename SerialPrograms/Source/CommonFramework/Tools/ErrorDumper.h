/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ErrorDumper_H
#define PokemonAutomation_ErrorDumper_H

#include <QImage>
#include "CommonFramework/Tools/Logger.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

namespace PokemonAutomation{


QString dump_image(
    Logger& logger,
    const ProgramInfo& program_info, const QString& label,
    const QImage& image
);


}
#endif
