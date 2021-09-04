/*  Program Notifications
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_ProgramNotifications_H
#define PokemonAutomation_ProgramNotifications_H

#include <QString>
#include "CommonFramework/Tools/Logger.h"

namespace PokemonAutomation{


void send_program_error_notification(
    Logger* logger,
    const QString& program,
    const QString& message
);


}
#endif
