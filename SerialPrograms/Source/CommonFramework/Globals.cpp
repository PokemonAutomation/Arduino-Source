/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QFile>
#include "Globals.h"

namespace PokemonAutomation{

const bool IS_BETA_VERSION = true;
const int PROGRAM_VERSION_MAJOR = 0;
const int PROGRAM_VERSION_MINOR = 30;
const int PROGRAM_VERSION_PATCH = 5;

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


const std::string SETTINGS_PATH = "UserSettings/";
const std::string SCREENSHOTS_PATH = "Screenshots/";
const std::string& RESOURCE_PATH(){
    static std::string path = get_resource_path();
    return path;
}
const std::string& TRAINING_PATH(){
    static std::string path = get_training_path();
    return path;
}



}

