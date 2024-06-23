/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "Globals.h"

namespace PokemonAutomation{


//
//  ATTENTION!!!
//
//  If you are building from source, do not change any of these version numbers
//  or tags. When filing reports or asking for support, we (the developers) need
//  to know exactly what you are running. Changing these values can lead to
//  misleading version information.
//

const bool IS_BETA_VERSION = true;
const int PROGRAM_VERSION_MAJOR = 0;
const int PROGRAM_VERSION_MINOR = 49;
const int PROGRAM_VERSION_PATCH = 11;

const std::string PROGRAM_VERSION_BASE =
    "v" + std::to_string(PROGRAM_VERSION_MAJOR) +
    "." + std::to_string(PROGRAM_VERSION_MINOR) +
    "." + std::to_string(PROGRAM_VERSION_PATCH);

#ifdef PA_OFFICIAL
const std::string PROGRAM_VERSION = IS_BETA_VERSION
    ? PROGRAM_VERSION_BASE + "-beta"
    : PROGRAM_VERSION_BASE;
#else
const std::string PROGRAM_VERSION = PROGRAM_VERSION_BASE + "-user";
#endif




const std::string PROGRAM_NAME = "Pok\u00e9mon Automation";

const std::string DISCORD_LINK = "discord.gg/PokemonAutomation";
const std::string DISCORD_LINK_URL = "https://discord.gg/cQ4gWxN";
const std::string ONLINE_DOC_URL = "https://github.com/PokemonAutomation/";
const std::string PROJECT_GITHUB = "github.com/PokemonAutomation";
const std::string PROJECT_GITHUB_URL = "https://github.com/PokemonAutomation/";
const std::string PROJECT_SOURCE_URL = "https://github.com/PokemonAutomation/Arduino-Source/";


namespace{

std::string get_resource_path(){
    //  Find the resource directory.
    QString path = QCoreApplication::applicationDirPath();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/Resources/";
        QFile file(try_path);
//        cout << path.toUtf8().data() << endl;
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (QCoreApplication::applicationDirPath() + "/../Resources/").toStdString();
}
std::string get_training_path(){
    //  Find the resource directory.
    QString path = QCoreApplication::applicationDirPath();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/TrainingData/";
        QFile file(try_path);
//        cout << path.toUtf8().data() << endl;
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (QCoreApplication::applicationDirPath() + "/../TrainingData/").toStdString();
}

std::string get_runtime_base_path(){
    QString application_dir_path = qApp->applicationDirPath();
    if (application_dir_path.endsWith(".app/Contents/MacOS")){
        // a macOS bundle. Change working directory to the folder that hosts the .app folder.
        QString app_bundle_path = application_dir_path.chopped(15);
        QString base_folder_path = QFileInfo(app_bundle_path).dir().absolutePath();
        return base_folder_path.toStdString() + "/";
    }
    return "./";
}
const std::string& RUNTIME_BASE_PATH(){
    static std::string path = get_runtime_base_path();
    return path;
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

const std::string& SETTINGS_PATH(){
    static std::string path = get_setting_path();
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
const std::string& TRAINING_PATH(){
    static std::string path = get_training_path();
    return path;
}



}

