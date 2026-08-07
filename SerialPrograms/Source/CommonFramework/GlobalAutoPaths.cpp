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
#if defined(__APPLE__)
#include <CoreFoundation/CFBundle.h>
#endif
#include "GlobalAutoPaths.h"

namespace PokemonAutomation{


#if defined(__APPLE__)
static std::string g_startup_profile;

void set_startup_profile(int& argc, char* argv[]){
    for (int i = 1; i + 1 < argc; i++){
        if (strcmp(argv[i], "--profile") == 0){
            QString profile = QString::fromUtf8(argv[i + 1]);
            for (QChar& c : profile){
                if (!c.isLetterOrNumber() && c != u'_' && c != u'-') c = u'_';
            }
            g_startup_profile = profile.toStdString();
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
std::string get_unittest_resource_path(){
    //  Find the resource directory.

    //  Try the intended folder name first.
    QString base = get_application_base_dir_path();
    QString path = base;
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/UnitTestResources/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }

    //  Now try with the old command-line folder.
    path = base;
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/CommandLineTests/";
        QFile file(try_path);
        if (file.exists()){
            return try_path.toStdString();
        }
        path += "/..";
    }

    return (base + "/UnitTestResources/").toStdString();
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
    if (!g_startup_profile.empty()){
        appSupportPath += "/Profiles/" + QString::fromStdString(g_startup_profile);
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
