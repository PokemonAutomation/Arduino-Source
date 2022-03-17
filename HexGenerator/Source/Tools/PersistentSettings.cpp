/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "Tools.h"
#include "PersistentSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const QString VERSION = "v0.5.18";
const QString DISCORD = "https://discord.gg/cQ4gWxN";
const QString GITHUB_REPO = "https://github.com/PokemonAutomation/";

const QString SETTINGS_NAME = "Settings.json";
const QString CONFIG_FOLDER_NAME = "GeneratorConfig";
const QString SOURCE_FOLDER_NAME = "NativePrograms";
const QString LOG_FOLDER_NAME = "Logs";


PersistentSettings settings;


void PersistentSettings::determine_paths(){
    path = QCoreApplication::applicationDirPath() + "/";

    for (size_t c = 0; c < 3; c++){
        cout << path.toUtf8().data() << endl;
//        QFileInfo info(path + SETTINGS_NAME);
//        if (info.exists() && info.isFile()){
//            return;
//        }
        QDir basedir(path);

        if (!QDir(path + LOG_FOLDER_NAME).exists()){
            basedir.mkdir(LOG_FOLDER_NAME);
        }
        if (QDir(path + CONFIG_FOLDER_NAME).exists()){
            return;
        }
        path += "../";
    }

    path = "";
//    throw StringExceptionQt("Unable to find working directory.");
}
void PersistentSettings::load(){
    determine_paths();
    cout << ("Root Path: " + path).toUtf8().data() << endl;

    try{
        QString full_path = path + SETTINGS_NAME;
        QJsonDocument doc = read_json_file(full_path);
        if (!doc.isObject()){
            throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid settings file.", full_path.toStdString());
        }

        QJsonObject root = doc.object();

        json_get_int(board_index, root, "Board", 0, 3);

    }catch (const Exception& e){
        std::cout << std::string("Error Parsing ") + SETTINGS_NAME.toUtf8().data() + ": " + e.message() << std::endl;
    }
}


void PersistentSettings::write() const{
    QJsonObject root;
    root.insert("Board", QJsonValue((int)board_index));
    write_json_file("Settings.json", QJsonDocument(root));
}


}

