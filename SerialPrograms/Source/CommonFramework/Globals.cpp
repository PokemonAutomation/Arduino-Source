/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QFile>
#include "Globals.h"

namespace PokemonAutomation{

const std::string PROGRAM_VERSION_BASE = "v0.24.8";

#ifdef PA_OFFICIAL
const std::string PROGRAM_VERSION = PROGRAM_VERSION_BASE;
#else
const std::string PROGRAM_VERSION = PROGRAM_VERSION_BASE + "u";
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


const std::string& RESOURCE_PATH(){
    static std::string path = get_resource_path();
    return path;
}
const std::string& TRAINING_PATH(){
    static std::string path = get_training_path();
    return path;
}



}

