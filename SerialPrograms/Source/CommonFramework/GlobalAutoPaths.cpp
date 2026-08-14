/*  Globals Auto-Paths
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Filesystem/Filesystem.h"
#include "GlobalAutoPaths.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


namespace{


std::string get_resource_path(){
    //  Find the resource directory.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "Resources/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }
    return (base / "Resources/").string_slash_normalized();
}
std::string get_unittest_resource_path(){
    //  Find the unit test resource directory.

    //  Try the intended folder name first.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "UnitTestResources/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    //  Now try with the old command-line folder.
    path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "CommandLineTests/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    return (base / "UnitTestResources/").string_slash_normalized();
}

std::string get_training_path(){
    //  Find the training data directory.
    Filesystem::Path base = Filesystem::application_install_path();
    Filesystem::Path path = base;
    for (size_t c = 0; c < 5; c++){
        Filesystem::Path try_path = path / "TrainingData/";
        if (exists(try_path)){
            return try_path.string_slash_normalized();
        }
        path = path / "..";
    }

    return (base / "TrainingData/").string_slash_normalized();
}

std::string get_setting_path(){
    return RUNTIME_BASE_PATH() + "UserSettings/";
}
std::string get_screenshot_path(){
    return RUNTIME_BASE_PATH() + "Screenshots/";
}
std::string get_debug_path(){
    return RUNTIME_BASE_PATH() + "DebugDumps/";
}
std::string get_error_path(){
    return RUNTIME_BASE_PATH() + "ErrorDumps/";
}
std::string get_user_file_path(){
    return RUNTIME_BASE_PATH();
}

} // anonymous namespace




const std::string& RUNTIME_BASE_PATH(){
    static std::string path = Filesystem::application_scratch_path().string();
    return path;
}

const std::string& SETTINGS_PATH(){
    static std::string path = get_setting_path();
    return path;
}
const std::string& PROGRAM_SETTING_JSON_PATH(){
    static std::string path = SETTINGS_PATH() + Filesystem::application_binary_name().replace_extension().string() + "-Settings.json";
    return path;
}
const std::string& SCREENSHOTS_PATH(){
    static std::string path = get_screenshot_path();
    return path;
}
const std::string& DEBUG_PATH(){
    static std::string path = get_debug_path();
    return path;
}
const std::string& ERROR_PATH(){
    static std::string path = get_error_path();
    return path;
}
const std::string& USER_FILE_PATH(){
    static std::string path = get_user_file_path();
    return path;
}
const std::string& RESOURCE_PATH(){
    static std::string path = get_resource_path();
    return path;
}
const std::string& DOWNLOADED_RESOURCE_PATH(){
    static std::string path = RUNTIME_BASE_PATH() + "DownloadedResources/";
    return path;
}
const std::string& UNIT_TEST_RESOURCE_PATH(){
    static std::string path = get_unittest_resource_path();
    return path;
}





const std::string& TRAINING_PATH(){
    static std::string path = get_training_path();
    return path;
}
const std::string& ML_ANNOTATION_PATH(){
    static const std::string path = RUNTIME_BASE_PATH() + "DataAnnotation/";
    return path;
}
const std::string& ML_MODEL_CACHE_PATH(){
    static const std::string path = RUNTIME_BASE_PATH() + "ModelCache/";
    return path;
}




}
