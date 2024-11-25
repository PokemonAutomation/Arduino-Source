/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#ifndef PokemonAutomation_Globals_H
#define PokemonAutomation_Globals_H

#include <chrono>
#include <string>

namespace PokemonAutomation{

extern const bool IS_BETA_VERSION;

extern const int PROGRAM_VERSION_MAJOR;
extern const int PROGRAM_VERSION_MINOR;
extern const int PROGRAM_VERSION_PATCH;
extern const std::string PROGRAM_VERSION_BASE;
extern const std::string PROGRAM_VERSION;

extern const std::string PROGRAM_NAME;

extern const std::string DISCORD_LINK;
extern const std::string DISCORD_LINK_URL;
extern const std::string ONLINE_DOC_URL;
extern const std::string PROJECT_GITHUB;
extern const std::string PROJECT_GITHUB_URL;
extern const std::string PROJECT_SOURCE_URL;

extern const std::string COMPILER_VERSION;

const auto SERIAL_REFRESH_RATE = std::chrono::milliseconds(1000);

// Folder path (end with "/") to hold program setting files.
const std::string& SETTINGS_PATH();

// Folder path (end with "/") to hold screenshots from the program "Screenshot" button.
const std::string& SCREENSHOTS_PATH();

// Folder path (end with "/") to hold debugging images and other debugging files
const std::string& DEBUG_PATH();

// Folder path (end with "/") to hold error images and other related files here. Useful for debugging the errors.
const std::string& ERROR_PATH();

// Folder path (end with "/") that holds various user genereated files.
// e.g. for a program that records and dumps screenshots, the saved images can go to USER_FILE_PATH()/ScreenshotDumper.
const std::string& USER_FILE_PATH();

// Resource folder path. Resources include JSON files, images, sound files and others required by
// various automation programs.
const std::string& RESOURCE_PATH();
// Hold ML trainign data.
const std::string& TRAINING_PATH();


enum class ProgramState{
    NOT_READY,
    STOPPED,
    RUNNING,
    STOPPING,
};

enum class FeedbackType{
    NONE,
    OPTIONAL_,  //  Naming conflict with macro.
    REQUIRED,
    VIDEO_AUDIO,
};



}
#endif
