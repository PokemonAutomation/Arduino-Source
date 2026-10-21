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

// Use Qt to setup permissions of cameras and microphones on macOS.
// See https://www.qt.io/blog/permission-apis-in-qt-6.5
void set_permissions(QObject& object);

// Return true if there is the required Resources/ folder.
// When the folder is missing, it will also pop a critical error message box
bool check_resource_folder(Logger& logger);

}
#endif
