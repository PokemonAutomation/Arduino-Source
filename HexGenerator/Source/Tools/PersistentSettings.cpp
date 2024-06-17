/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QDir>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Tools.h"
#include "PersistentSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace HexGenerator{


const std::string VERSION = "v0.5.24";
const std::string DISCORD = "https://discord.gg/cQ4gWxN";
const std::string GITHUB_REPO = "https://github.com/PokemonAutomation/";

const std::string SETTINGS_NAME = "Settings.json";
const std::string CONFIG_FOLDER_NAME = "GeneratorConfig";
const std::string SOURCE_FOLDER_NAME = "NativePrograms";
const std::string LOG_FOLDER_NAME = "Logs";


PersistentSettings settings;


void PersistentSettings::determine_paths(){
    path = QCoreApplication::applicationDirPath().toStdString() + "/";

    for (size_t c = 0; c < 3; c++){
        cout << path << endl;
//        QFileInfo info(path + SETTINGS_NAME);
//        if (info.exists() && info.isFile()){
//            return;
//        }
        QDir basedir(QString::fromStdString(path));

        if (!QDir(QString::fromStdString(path + LOG_FOLDER_NAME)).exists()){
            basedir.mkdir(QString::fromStdString(LOG_FOLDER_NAME));
        }
        if (QDir(QString::fromStdString(path + CONFIG_FOLDER_NAME)).exists()){
            return;
        }
        path += "../";
    }

    path = "";
//    throw StringExceptionQt("Unable to find working directory.");
}
void PersistentSettings::load(){
    determine_paths();
    cout << "Root Path: " + path << endl;

    try{
        std::string full_path = path + SETTINGS_NAME;
        JsonValue json = load_json_file(full_path);
        JsonObject& obj = json.to_object_throw(full_path);
        obj.read_integer(board_index, "Board", 0, 3);
    }catch (const Exception& e){
        std::cout << std::string("Error Parsing ") + SETTINGS_NAME + ": " + e.message() << std::endl;
    }
}


void PersistentSettings::write() const{
    JsonObject root;
    root["Board"] = board_index;
    root.dump("Settings.json");
}



}
}

