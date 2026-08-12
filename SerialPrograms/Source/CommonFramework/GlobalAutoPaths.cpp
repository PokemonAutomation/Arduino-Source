/*  Globals Auto-Paths
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QString>
#include <QFile>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "Common/Cpp/Filesystem/Filesystem.h"
#include "GlobalAutoPaths.h"

#ifdef __linux__
#include <cstdlib>
#include <fstream>
#include "Common/Cpp/Strings/StringTools.h"
#include "Common/Cpp/Filesystem/FileIO.h"
#endif

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{





#if defined(__APPLE__)
static std::string g_startup_profile;

void set_startup_profile(int& argc, char* argv[]){
    for (int i = 1; i + 1 < argc; i++){
        if (strcmp(argv[i], "--profile") == 0){
            std::string profile = argv[i + 1];
            for (char c : profile){
                if (!(std::isalpha(c) || std::isdigit(c)) && c != u'_' && c != u'-') c = u'_';
            }
            g_startup_profile = std::move(profile);
            // Shift everything after --profile <name> down by 2.
            for (int j = i; j + 2 < argc; j++){
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            return;
        }
    }
}

const std::string& STARTUP_PROFILE(){
    return g_startup_profile;
}
#endif

namespace{




Filesystem::Path get_application_base_dir_path(){
#if defined(__linux__)
    // Check for AppImage environment variables to find the root directory, if running as an AppImage.
    // PA_APPIMAGE_DIR is set by Azure via a patched AppRun script.
    {
        char* dir = std::getenv("PA_APPIMAGE_DIR");
        if (dir != nullptr){
            return dir;
        }
    }
    {
        char* path = std::getenv("APPIMAGE");
        if (path != nullptr){
            return std::filesystem::absolute(path).parent_path().lexically_normal();
        }
    }
    {
        char* app_dir = std::getenv("APPDIR");
        if (app_dir != nullptr){
            std::ifstream mount_info(Filesystem::Path("/proc/self/mountinfo").stdpath());
            if (mount_info.is_open()){
                std::string line;
                while (std::getline(mount_info, line)){
                    size_t dash_sep = line.find(" - ");
                    if (dash_sep == std::string::npos) {
                        continue;
                    }

                    // 1. Extract and tokenize the "pre" segment (before " - ")
                    std::vector<std::string> pre;
                    {
                        std::string pre_str = line.substr(0, dash_sep);
                        std::stringstream ss(pre_str);
                        std::string token;
                        while (ss >> token) {
                            pre.push_back(token);
                        }
                    }

                    // 2. Extract and tokenize the "post" segment (after " - ")
                    std::vector<std::string> post;
                    {
                        std::string post_str = line.substr(dash_sep + 3);
                        std::stringstream ss(post_str);
                        std::string token;
                        while (ss >> token) {
                            post.push_back(token);
                        }
                    }

                    // 3. Validation bounds check
                    if (pre.size() < 5 || post.size() < 2){
                        continue;
                    }

                    // 4. Clean out octal space encodings using standard string operations
                    std::string mount_point = StringTools::replace(pre[4], "\\040", " ");
                    std::string source = StringTools::replace(post[1], "\\040", " ");

                    if (mount_point == app_dir && source.ends_with(".AppImage")){
                        Filesystem::Path dir_path = Filesystem::Path(source).parent_path();
                        Filesystem::Path abs_path = Filesystem::absolute(dir_path);
                        return abs_path.lexically_normal();
                    }
                }
            }
        }
    }
#endif
    return Filesystem::application_install_path();
}
std::string get_resource_path(){
    //  Find the resource directory.
    Filesystem::Path base = get_application_base_dir_path();
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
    Filesystem::Path base = get_application_base_dir_path();
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
    Filesystem::Path base = get_application_base_dir_path();
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

std::string get_runtime_base_path(){
#if defined(__APPLE__)
    // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) returns
    // "/Users/$USERNAME/Library/Application Support/SerialPrograms"
    QString appSupportPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!g_startup_profile.empty()){
        appSupportPath += "/Profiles/" + QString::fromStdString(g_startup_profile);
    }
    QDir().mkpath(appSupportPath);
    return appSupportPath.toStdString() + "/";
#else
    return Filesystem::application_scratch_path().string_slash_normalized() + "/";
#endif
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
    static std::string path = get_runtime_base_path();
    return path;
}

const std::string& SETTINGS_PATH(){
    static std::string path = get_setting_path();
    return path;
}
const std::string& PROGRAM_SETTING_JSON_PATH(){
    static std::string path = SETTINGS_PATH() + QCoreApplication::applicationName().toStdString() + "-Settings.json";
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
