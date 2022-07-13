/*  Error Dumper
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ErrorDumper_H
#define PokemonAutomation_ErrorDumper_H

#include <QImage>
#include "CommonFramework/Logging/LoggerQt.h"
#include "CommonFramework/Notifications/ProgramNotifications.h"

namespace PokemonAutomation{


std::string dump_image(
    LoggerQt& logger,
    const ProgramInfo& program_info, const std::string& label,
    const QImage& image
);


}
#endif
