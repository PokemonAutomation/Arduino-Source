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
