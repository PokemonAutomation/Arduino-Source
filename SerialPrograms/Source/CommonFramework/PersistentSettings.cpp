/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
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
    root.insert("04-DeveloperMode", QJsonValue(developer_mode));
    root.insert("05-LogEverything", QJsonValue(log_everything));
//    root.insert("06-ResourcePath", QJsonValue(resource_path));

    root.insert("10-INSTANCE_NAME", INSTANCE_NAME);

    root.insert("20-DISCORD_WEBHOOK_URL", DISCORD_WEBHOOK_URL);
    root.insert("21-DISCORD_USER_ID", DISCORD_USER_ID);
    root.insert("22-DISCORD_USER_SHORT_NAME", DISCORD_USER_SHORT_NAME);

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
    json_get_bool(developer_mode, root, "04-DeveloperMode");
    json_get_bool(log_everything, root, "05-LogEverything");
//    json_get_string(resource_path, root, "06-ResourcePath");

    json_get_string(INSTANCE_NAME, root, "10-INSTANCE_NAME");

    json_get_string(DISCORD_WEBHOOK_URL, root, "20-DISCORD_WEBHOOK_URL");
    json_get_string(DISCORD_USER_ID, root, "21-DISCORD_USER_ID");
    json_get_string(DISCORD_USER_SHORT_NAME, root, "22-DISCORD_USER_SHORT_NAME");

    NintendoSwitch::set_keyboard_mapping(json_get_array_nothrow(root, "50-SwitchKeyboardMapping"));

    panels = json_get_object_nothrow(root, "99-Panels");
}



}

