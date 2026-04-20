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
#if defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#endif
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

#ifndef PA_IS_BETA
#define PA_IS_BETA true
#endif

#ifndef PA_VERSION_MAJOR
#define PA_VERSION_MAJOR 0
#endif

#ifndef PA_VERSION_MINOR
#define PA_VERSION_MINOR 63
#endif

#ifndef PA_VERSION_PATCH
#define PA_VERSION_PATCH 7
#endif

const bool IS_BETA_VERSION = PA_IS_BETA;
const int PROGRAM_VERSION_MAJOR = PA_VERSION_MAJOR;
const int PROGRAM_VERSION_MINOR = PA_VERSION_MINOR;
const int PROGRAM_VERSION_PATCH = PA_VERSION_PATCH;

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

const std::string ONLINE_DOC_URL_BASE = "https://pokemonautomation.github.io/";
const std::string PROJECT_SOURCE_URL = "https://github.com/PokemonAutomation/Arduino-Source/";
const std::string RESOURCES_URL_BASE = "https://github.com/PokemonAutomation/Packages/";



//  This the URL that we display. We don't actually use this for linking.
const std::string GITHUB_LINK_TEXT = "pokemonautomation.github.io";

//  This is the URL that we actually link to.
const std::string GITHUB_LINK_URL = "https://pokemonautomation.github.io";



//  URL to display. (the vanity link)
//  We don't actually use this URL for linking since the vanity link will go
//  away if we lose too many nitro boosts.
const std::string DISCORD_LINK_TEXT = "discord.gg/PokemonAutomation";

//  URL to use inside the program.
const std::string DISCORD_LINK_URL_PROGRAM = "https://discord.gg/BSjDp27";

//  URL to use in the Discord notifications/embeds.
const std::string DISCORD_LINK_URL_EMBED = "https://discord.gg/xMJcveK";

//  URL to use in the DiscordSocialSDK integration.
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

#if defined(__APPLE__)
static QString g_startup_profile;

void set_startup_profile(int& argc, char* argv[]){
    for (int i = 1; i + 1 < argc; i++){
        if (strcmp(argv[i], "--profile") == 0){
            QString profile = QString::fromUtf8(argv[i + 1]);
            for (QChar& c : profile){
                if (!c.isLetterOrNumber() && c != u'_' && c != u'-') c = u'_';
            }
            g_startup_profile = profile;
            // Shift everything after --profile <name> down by 2.
            for (int j = i; j + 2 < argc; j++){
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            return;
        }
    }
}
#endif

namespace{

QString get_application_base_dir_path(){
    QString application_dir_path = qApp->applicationDirPath();
#if defined(__APPLE__)
    //  Use CFBundle to find the .app bundle path. Change working directory to the folder that hosts the .app bundle.
    CFURLRef bundleURL = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    if (bundleURL){
        CFStringRef cfPath = CFURLCopyFileSystemPath(bundleURL, kCFURLPOSIXPathStyle);
        CFRelease(bundleURL);
        if (cfPath){
            QString bundlePath = QDir::cleanPath(QString::fromCFString(cfPath));
            CFRelease(cfPath);
            if (bundlePath.endsWith(".app")){
                return QFileInfo(bundlePath).dir().absolutePath();
            }
        }
    }
#elif defined(__linux__)
    // Check for AppImage environment variables to find the root directory, if running as an AppImage.
    // PA_APPIMAGE_DIR is set by Azure via a patched AppRun script.
    QByteArray dir = qgetenv("PA_APPIMAGE_DIR");
    if (!dir.isEmpty()){
        return QString::fromUtf8(dir);
    }
    QByteArray path = qgetenv("APPIMAGE");
    if (!path.isEmpty()){
        return QDir::cleanPath(QFileInfo(QString::fromUtf8(path)).dir().absolutePath());
    }
    QByteArray appDirBytes = qgetenv("APPDIR");
    if (!appDirBytes.isEmpty()){
        QString appDir = QString::fromUtf8(appDirBytes);
        QFile mountinfo(QStringLiteral("/proc/self/mountinfo"));
        if (mountinfo.open(QIODevice::ReadOnly | QIODevice::Text)){
            while (!mountinfo.atEnd()){
                QString line = QString::fromUtf8(mountinfo.readLine()).trimmed();
                int dashSep = line.indexOf(QStringLiteral(" - "));
                if (dashSep < 0){
                    continue;
                }
                QStringList pre = line.left(dashSep).split(u' ', Qt::SkipEmptyParts);
                QStringList post = line.mid(dashSep + 3).split(u' ', Qt::SkipEmptyParts);
                if (pre.size() < 5 || post.size() < 2){
                    continue;
                }
                QString mountPoint = pre[4].replace(QStringLiteral("\\040"), QStringLiteral(" "));
                QString source = post[1].replace(QStringLiteral("\\040"), QStringLiteral(" "));
                if (mountPoint == appDir && source.endsWith(QStringLiteral(".AppImage"))){
                    return QDir::cleanPath(QFileInfo(source).dir().absolutePath());
                }
            }
        }
    }
#endif
    return application_dir_path;
}
std::string get_resource_path(){
    //  Find the resource directory.
    QString base = get_application_base_dir_path();
    QString path = base;
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/Resources/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (base + "/Resources/").toStdString();
}
std::string get_training_path(){
    //  Find the training data directory.
    QString base = get_application_base_dir_path();
    QString path = base;
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/TrainingData/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }
    return (base + "/TrainingData/").toStdString();
}

std::string get_runtime_base_path(){
#if defined(__APPLE__)
    // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) returns
    // "/Users/$USERNAME/Library/Application Support/SerialPrograms"
    QString appSupportPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!g_startup_profile.isEmpty()){
        appSupportPath += "/Profiles/" + g_startup_profile;
    }
    QDir().mkpath(appSupportPath);
    return appSupportPath.toStdString() + "/";
#else
    return "./";
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

#if 0
// Program executable path information
namespace {
    std::string g_program_absolute_path;
    std::string g_program_filename;
    std::string g_program_basename;
}

void set_program_path(const char* argv0){
    if (argv0 != nullptr){
        std::filesystem::path program_path(argv0);
        g_program_absolute_path = std::filesystem::absolute(program_path).string();
        g_program_filename = program_path.filename().string();
        g_program_basename = program_path.stem().string();
    }
}
const std::string& PROGRAM_ABSOLUTE_PATH(){ return g_program_absolute_path }
const std::string& PROGRAM_FILENAME(){ return g_program_filename; }
const std::string& PROGRAM_BASENAME(){ return g_program_basename; }
#endif

}

