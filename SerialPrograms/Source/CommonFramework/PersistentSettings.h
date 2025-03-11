/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */


#ifndef PokemonAutomation_PersistentSettings_H
#define PokemonAutomation_PersistentSettings_H

#include "Common/Cpp/Json/JsonObject.h"

namespace PokemonAutomation{

// Global setting of the whole program.
// The setting is stored in the local folder, named as SerialPrograms-Settings.json.
// The settings json has three fields:
// - "20-GlobalSettings": settings for CommonFramework/GlobalSettingsPanel.h: GlobalSetting,
//   global settings like Discord settings, hardware instruction set, thread priority and debugging.
// - "50-SwitchKeyboardMapping": keyboard mapping.
// - "99-Panels": settings for all the programs listed in the program panels.
//   Access via PersistentSettings::panels.
class PersistentSettings{
public:
    PersistentSettings();

    // Write settings to the json file.
    void write() const;
    // Load settings from the json file.
    void read();

public:
    JsonObject panels;
};

// Return the singleton PersistentSettings.
PersistentSettings& PERSISTENT_SETTINGS();



}


#endif
