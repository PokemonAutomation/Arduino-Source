/*  Setup Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_SetupSettings_H
#define PokemonAutomation_SetupSettings_H

#include <string>

class QObject;

namespace PokemonAutomation{

class Logger;

// Move user setting JSON file from old location to the new one
// return true if there is no need for migration or the migration is successful
// return false if migration fails. In this case it will also pop a critical error message box
bool migrate_settings(Logger& logger, const std::string& file_name);

// Move stats file from old location to the new one
// return true if there is no need for migration or the migration is successful
// return false if migration fails. In this case it will also pop a critical error message box
bool migrate_stats(Logger& logger);

// Use Qt to setup permissions of cameras and microphones on macOS.
// See https://www.qt.io/blog/permission-apis-in-qt-6.5
void set_permissions(QObject& object);

// Return true if there is the required Resources/ folder.
// When the folder is missing, it will also pop a critical error message box
bool check_resource_folder(Logger& logger);

}
#endif
