/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include <QStandardPaths>
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
const int PROGRAM_VERSION_MINOR = 57;
const int PROGRAM_VERSION_PATCH = 3;

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

const std::string ONLINE_DOC_URL_BASE = "https://github.com/PokemonAutomation/";
const std::string PROJECT_SOURCE_URL = "https://github.com/PokemonAutomation/Arduino-Source/";
const std::string RESOURCES_URL_BASE = "https://github.com/PokemonAutomation/Packages/";



//  This the URL that we display. We don't actually use this for linking.
const std::string GITHUB_LINK_TEXT = "github.com/PokemonAutomation";

//  This is the URL that we actually link to.
const std::string GITHUB_LINK_URL = "https://github.com/PokemonAutomation/About/blob/master/README.md";



//  URL to display. (the vanity link)
//  We don't actually use this URL for linking since the vanity link will go
//  away if we lose too many nitro boosts.
const std::string DISCORD_LINK_TEXT = "discord.gg/PokemonAutomation";

//  URL to use inside the program.
const std::string DISCORD_LINK_URL_PROGRAM = "https://discord.gg/BSjDp27";

//  URL to use in the Discord notifications/embeds.
const std::string DISCORD_LINK_URL_EMBED = "https://discord.gg/xMJcveK";

// URL to use in the DiscordSocialSDK integration.
const std::string DISCORD_LINK_URL_SDK = "https://discord.gg/gn9YEyjjAV";



#if 0
#elif __INTEL_LLVM_COMPILER
const std::string COMPILER_VERSION = "ICX " + std::to_string(__VERSION__);
#elif __INTEL_COMPILER
const std::string COMPILER_VERSION = "ICC " + std::to_string(__INTEL_COMPILER) + "." + std::to_string(__INTEL_COMPILER_UPDATE);
#elif _MSC_VER
const std::string COMPILER_VERSION = "MSVC " + std::to_string(_MSC_FULL_VER);
#elif __clang__
const std::string COMPILER_VERSION = "Clang " + std::string(__clang_version__);
#elif __GNUC__
const std::string COMPILER_VERSION = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#else
const std::string COMPILER_VERSION = "Unknown Compiler";
#endif



const size_t LOG_HISTORY_LINES = 10000;


namespace{

QString get_application_base_dir_path(){
    QString application_dir_path = qApp->applicationDirPath();
    if (application_dir_path.endsWith(".app/Contents/MacOS")){
        // a macOS bundle. Change working directory to the folder that hosts the .app folder.
        QString app_bundle_path = application_dir_path.chopped(15);
        QString base_folder_path = QFileInfo(app_bundle_path).dir().absolutePath();
        return base_folder_path;
    }
    return application_dir_path;
}
std::string get_resource_path(){
    //  Find the resource directory.
    QString path = get_application_base_dir_path();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/Resources/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (QCoreApplication::applicationDirPath() + "/../Resources/").toStdString();
}
std::string get_training_path(){
    //  Find the training data directory.
    QString path = get_application_base_dir_path();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/TrainingData/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (QCoreApplication::applicationDirPath() + "/../TrainingData/").toStdString();
}

std::string get_runtime_base_path(){
    //  On MacOS, find the writable application support directory
    if (QSysInfo::productType() == "macos" || QSysInfo::productType() == "osx"){
        QString appSupportPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir(appSupportPath);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        return appSupportPath.toStdString() + "/";
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

