/*  Persistent Settings
 *
 *  From: https://github.com/Mysticial/Pokemon-Automation-SwSh-Arduino-Scripts
 *
 */

#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include "Common/Qt/QtJsonTools.h"
#include "Common/Qt/StringException.h"
#include "Tools.h"
#include "PersistentSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const QString VERSION = "v0.3.9";
const QString DISCORD = "https://discord.gg/cQ4gWxN";
const QString GITHUB_REPO = "https://github.com/PokemonAutomation/SwSh-Arduino";

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
//    throw StringException("Unable to find working directory.");
}
void PersistentSettings::load(){
    determine_paths();
    cout << ("Root Path: " + path).toUtf8().data() << endl;

    try{
        QJsonDocument doc = read_json_file(path + SETTINGS_NAME);
        if (!doc.isObject()){
            throw StringException("Invalid settings file.");
        }

        QJsonObject root = doc.object();

        board_index = json_get_int(root, "Board");
        if (board_index >= 4){
            board_index = 0;
        }

    }catch (const StringException& str){
        std::cout << ("Error Parsing " + SETTINGS_NAME + ": " + str.message()).toUtf8().data() << std::endl;
    }
}


void PersistentSettings::write() const{
    QJsonObject root;
    root.insert("Board", QJsonValue((int)board_index));
    write_json_file("Settings.json", QJsonDocument(root));
}


}

