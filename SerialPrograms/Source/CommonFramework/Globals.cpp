/*  Globals
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QFile>
#include "Globals.h"

namespace PokemonAutomation{

const QString PROGRAM_VERSION_BASE = "v0.14.4";

#ifdef PA_OFFICIAL
const QString PROGRAM_VERSION = PROGRAM_VERSION_BASE;
#else
const QString PROGRAM_VERSION = PROGRAM_VERSION_BASE + "u";
#endif

const QString DISCORD_LINK = "discord.gg/PokemonAutomation";
const QString DISCORD_LINK_URL = "https://discord.gg/cQ4gWxN";
const QString ONLINE_DOC_URL = "https://github.com/PokemonAutomation/";
const QString PROJECT_GITHUB = "github.com/PokemonAutomation";
const QString PROJECT_GITHUB_URL = "https://github.com/PokemonAutomation/";
const QString PROJECT_SOURCE_URL = "https://github.com/PokemonAutomation/Arduino-Source/";

const QString STRING_POKEBALL = QString("Pok") + QChar(0xe9) + " Ball";
const QString STRING_POKEMON = QString("Pok") + QChar(0xe9) + "mon";
const QString STRING_POKEDEX = QString("Pok") + QChar(0xe9) + "dex";
const QString STRING_POKEJOB = QString("Pok") + QChar(0xe9) + " Job";



QString get_resource_path(){
    //  Find the resource directory.
    QString path = QCoreApplication::applicationDirPath();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/Resources/";
        QFile file(try_path);
//        cout << path.toUtf8().data() << endl;
        if (file.exists()){
            return try_path;
        }
        path += "/..";
    }
    return QCoreApplication::applicationDirPath() + "/../Resources/";
}
QString get_training_path(){
    //  Find the resource directory.
    QString path = QCoreApplication::applicationDirPath();
    for (size_t c = 0; c < 5; c++){
        QString try_path = path + "/TrainingData/";
        QFile file(try_path);
//        cout << path.toUtf8().data() << endl;
        if (file.exists()){
            return try_path;
        }
        path += "/..";
    }
    return QCoreApplication::applicationDirPath() + "/../TrainingData/";
}


const QString& RESOURCE_PATH(){
    static QString path = get_resource_path();
    return path;
}
const QString& TRAINING_PATH(){
    static QString path = get_training_path();
    return path;
}



}

