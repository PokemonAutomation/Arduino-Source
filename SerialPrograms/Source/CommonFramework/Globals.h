/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#ifndef PokemonAutomation_Globals_H
#define PokemonAutomation_Globals_H

#include <string>

namespace PokemonAutomation{

extern const bool IS_BETA_VERSION;

extern const int PROGRAM_VERSION_MAJOR;
extern const int PROGRAM_VERSION_MINOR;
extern const int PROGRAM_VERSION_PATCH;
extern const std::string PROGRAM_VERSION_BASE;
extern const std::string PROGRAM_VERSION;

extern const std::string PROGRAM_NAME;

extern const std::string ONLINE_DOC_URL_BASE;
extern const std::string PROJECT_SOURCE_URL;
extern const std::string RESOURCES_URL_BASE;

extern const std::string GITHUB_LINK_TEXT;
extern const std::string GITHUB_LINK_URL;

extern const std::string DISCORD_LINK_TEXT;
extern const std::string DISCORD_LINK_URL_PROGRAM;
extern const std::string DISCORD_LINK_URL_EMBED;
extern const std::string DISCORD_LINK_URL_SDK;

extern const std::string COMPILER_VERSION;

extern const size_t LOG_HISTORY_LINES;

// Folder path (end with "/") to hold program setting files.
const std::string& SETTINGS_PATH();
// The setting JSON file path. This path is a child of the folder SETTINGS_PATH().
const std::string& PROGRAM_SETTING_JSON_PATH();

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
// Hold ML training data.
const std::string& TRAINING_PATH();

// Folder path (end with "/") to hold data annotation for ML labeling programs
const std::string& ML_ANNOTATION_PATH();
// Folder path (end with "/") to hold model cache for model inferences. This is only used on macOS
// for the Apple CoreML model acceleration framework can create model cache for faster model inference
// sessions.
const std::string& ML_MODEL_CACHE_PATH();


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
