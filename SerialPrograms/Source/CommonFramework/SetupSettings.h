/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SetupSettings_H
#define PokemonAutomation_SetupSettings_H

#include <string>

namespace PokemonAutomation{

class Logger;


bool migrate_settings(Logger& logger, std::string file_name);
bool migrate_stats(Logger& logger);


}
#endif
