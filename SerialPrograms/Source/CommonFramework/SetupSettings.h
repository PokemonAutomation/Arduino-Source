/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SetupSettings_H
#define PokemonAutomation_SetupSettings_H

#include "CommonFramework/Logging/Logger.h"

namespace PokemonAutomation{

bool setup_settings(Logger& logger = global_logger_tagged(), std::string file_name = std::string());



}
#endif
