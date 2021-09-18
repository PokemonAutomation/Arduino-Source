/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch/Framework/VirtualSwitchControllerMapping.h"
#include "PersistentSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


const std::set<std::string> TOKENS{
    "f6538243092d8a3b9959bca988f054e1670f57c7246df2cbba25c4df3fe7a4e7",
    "2d04af67f6520e3550842d7eeb292868c6d0d4809b607f5a454712023d8815e1",
    "475d0a0a305a02cbf8b602bd47c3b275dccd5ac19fbe480729804a8e4e360b71",
    "6643d9fe87b3e54dc75dfac8ac22f0cc8bd17f6a8a786debf5fc4c517ee65469",
    "8e48e38e49bffc8462ada9d2d9d850d5b3b5c9529d20978c09bc548bc9a614a4",
    "7694adee4419d62c6a923c4efc9e7b41def7b96bb84ea882701b0bf2e8c13bee",
};





PersistentSettings& PERSISTENT_SETTINGS(){
    static PersistentSettings settings;
    return settings;
}


PersistentSettings::PersistentSettings()
    : stats_file("PA-Stats.txt")
//    , window_size(1280, 720)
    , window_width(1280)
    , window_height(720)
    , naughty_mode(false)
    , developer_mode(false)
    , log_everything(false)
    , save_debug_images(true)
{
    //  Find the resource directory.
    QString path = QCoreApplication::applicationDirPath();
    for (size_t c = 0; c < 5; c++){
        QFile file(path + "/Resources/");
//        cout << path.toUtf8().data() << endl;
        if (file.exists()){
            resource_path = path + "/Resources/";
            training_data = path + "/TrainingData/";
            break;
        }
        path += "/..";
    }
    if (resource_path.isEmpty()){
        resource_path = QCoreApplication::applicationDirPath() + "/../Resources/";
        training_data = QCoreApplication::applicationDirPath() + "/../TrainingData/";
    }
//    cout << "resources = " << resource_path.toUtf8().data() << endl;
}



void PersistentSettings::write() const{
    QJsonObject root;
    root.insert("01-StatsFile", QJsonValue(stats_file));
    {
        QJsonArray res;
        res.append(QJsonValue((int)window_width));
        res.append(QJsonValue((int)window_height));
        root.insert("02-WindowSize", res);
    }
    root.insert("03-NaughtyMode", QJsonValue(naughty_mode));
    root.insert("05-LogEverything", QJsonValue(log_everything));
    root.insert("06-SaveDebugImages", QJsonValue(save_debug_images));
    root.insert("07-DeveloperToken", QJsonValue(developer_token));

    root.insert("10-DiscordSettings", discord_settings.to_json());

    root.insert("50-SwitchKeyboardMapping", NintendoSwitch::read_keyboard_mapping());

    root.insert("99-Panels", panels);
//    cout << QJsonDocument(panels).toJson().data() << endl;

    write_json_file(QCoreApplication::applicationFilePath() + "-Settings.json", QJsonDocument(root));
}
void PersistentSettings::read(){
    QJsonDocument doc = read_json_file(QCoreApplication::applicationFilePath() + "-Settings.json");
    if (!doc.isObject()){
        PA_THROW_ParseException("Invalid settings file.");
    }
    QJsonObject root = doc.object();
    json_get_string(stats_file, root, "01-StatsFile");

    {
        QJsonArray res = json_get_array_nothrow(root, "02-WindowSize");
        if (res.size() == 2){
            window_width = res[0].toInt(window_width);
            window_height = res[1].toInt(window_height);
//            window_size = QSize(
//                res[0].toInt(window_size.width()),
//                res[1].toInt(window_size.height())
//            );
        }
    }
    json_get_bool(naughty_mode, root, "03-NaughtyMode");
    json_get_bool(log_everything, root, "05-LogEverything");
    json_get_bool(save_debug_images, root, "06-SaveDebugImages");
    json_get_string(developer_token, root, "07-DeveloperToken");

    {
        std::string token = developer_token.toStdString();
        QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
        hash.addData(token.c_str(), token.size());
        developer_mode = TOKENS.find(hash.result().toHex().toStdString()) != TOKENS.end();
    }

    discord_settings.load_json(json_get_object_throw(root, "10-DiscordSettings"));

    NintendoSwitch::set_keyboard_mapping(json_get_array_nothrow(root, "50-SwitchKeyboardMapping"));

    panels = json_get_object_nothrow(root, "99-Panels");
}



}

