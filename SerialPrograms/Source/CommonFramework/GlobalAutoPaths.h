/*  Globals Auto-Paths
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_GlobalAutoPaths_H
#define PokemonAutomation_GlobalAutoPaths_H

#include <string>

namespace PokemonAutomation{


// Path to the parent folder that holds all other folders, e.g. settings folder, screenshot folder, etc.
const std::string& RUNTIME_BASE_PATH();


// Where the program stores user data (user setting JSON, bug reports,
// screenshots, etc).
enum class UserDataStorageLocation{
    USER_APP,   //  The user's application data folder of the operating system.
                //  This location survives program updates. e.g.
                //  - Windows: %APPDATA%/<program name>/
                //  - macOS: ~/Library/Application Support/<program name>/
                //  - Linux: ~/.local/share/<program name>/
    LOCAL,      //  The folder the program runs from (the current working
                //  directory; on macOS, the folder that hosts the app).
};

// The platform default storage location: LOCAL on Windows and Linux,
// USER_APP on macOS. This matches where the program stored user data before
// the storage location became user-selectable.
UserDataStorageLocation DEFAULT_USER_DATA_STORAGE_LOCATION();

// Human-readable name of a storage location, e.g. for logging.
const std::string& USER_DATA_STORAGE_LOCATION_DISPLAY_NAME(UserDataStorageLocation location);

// Base folder (end with "/") that holds user data when the storage location is LOCAL.
const std::string& LOCAL_DATA_BASE_PATH();
// Base folder (end with "/") that holds user data when the storage location is USER_APP.
const std::string& USER_APP_DATA_BASE_PATH();

// The storage location currently in use for the user setting JSON and bug
// reports. It is set from the "Data Storage" options in the Global Settings
// panel, and is initialized at startup (before the setting JSON file is read)
// by DETECT_SETTINGS_STORAGE_LOCATION().
UserDataStorageLocation SETTINGS_STORAGE_LOCATION();
void SET_SETTINGS_STORAGE_LOCATION(UserDataStorageLocation location);

// The storage location currently in use for screenshots. (See
// SETTINGS_STORAGE_LOCATION() for details.)
UserDataStorageLocation SCREENSHOTS_STORAGE_LOCATION();
void SET_SCREENSHOTS_STORAGE_LOCATION(UserDataStorageLocation location);

// Base folder (end with "/") for the user setting JSON and bug reports.
// Follows SETTINGS_STORAGE_LOCATION().
const std::string& USER_DATA_BASE_PATH();
// Base folder (end with "/") for screenshots. Follows SCREENSHOTS_STORAGE_LOCATION().
const std::string& SCREENSHOTS_BASE_PATH();

// Determine which storage location currently holds the newest user setting
// JSON file, and set SETTINGS_STORAGE_LOCATION() accordingly:
// - If only one of the two candidate setting JSON files exists, use that location.
// - If both exist, use the location of the one that was modified most recently.
//   (This happens when the user switches the storage location: the program
//   saves the settings at the new location while the older file remains at the
//   old location.)
// - If neither exists, use the platform default location.
// Call this at startup, before reading the setting JSON file.
void DETECT_SETTINGS_STORAGE_LOCATION();


// Folder path (end with "/") to hold program setting files.
const std::string& SETTINGS_PATH();
// The file name of the program setting JSON file, e.g. "SerialPrograms-Settings.json".
const std::string& PROGRAM_SETTING_JSON_FILE_NAME();
// The setting JSON file path. This path is a child of the folder SETTINGS_PATH().
const std::string& PROGRAM_SETTING_JSON_PATH();

// Folder path (end with "/") to hold screenshots from the program "Screenshot" button.
const std::string& SCREENSHOTS_PATH();

// Folder path (end with "/") to hold debugging images and other debugging files
const std::string& DEBUG_PATH();

// Folder path (end with "/") to hold error images and other related files here. Useful for debugging the errors.
const std::string& ERROR_PATH();

// Folder path (end with "/") that holds unsent error (bug) reports.
const std::string& UNSENT_ERROR_REPORTS_PATH();
// Folder path (end with "/") that holds sent error (bug) reports.
const std::string& SENT_ERROR_REPORTS_PATH();

// Folder path (end with "/") that holds various user genereated files.
// e.g. for a program that records and dumps screenshots, the saved images can go to USER_FILE_PATH()/ScreenshotDumper.
const std::string& USER_FILE_PATH();

// Resource folder path. Resources include JSON files, images, sound files and others required by
// various automation programs.
const std::string& RESOURCE_PATH();

// Folder path that holds Downloaded resources
const std::string& DOWNLOADED_RESOURCE_PATH();

// Folder path that holds the unit test resources.
const std::string& UNIT_TEST_RESOURCE_PATH();

// Hold ML training data.
const std::string& TRAINING_PATH();

// Folder path (end with "/") to hold data annotation for ML labeling programs
const std::string& ML_ANNOTATION_PATH();
// Folder path (end with "/") to hold model cache for model inferences. This is only used on macOS
// for the Apple CoreML model acceleration framework to create model cache for faster model inference
// sessions.
const std::string& ML_MODEL_CACHE_PATH();



}
#endif
