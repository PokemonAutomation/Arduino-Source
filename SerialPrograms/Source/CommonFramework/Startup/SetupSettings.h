/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_SetupSettings_H
#define PokemonAutomation_SetupSettings_H

#include <string>

class QObject;

namespace PokemonAutomation{

class Logger;


bool migrate_settings(Logger& logger, std::string file_name);
bool migrate_stats(Logger& logger);

// Use Qt to setup permissions of cameras and microphones on macOS.
// See https://www.qt.io/blog/permission-apis-in-qt-6.5
void set_permissions(QObject& object);


}
#endif
