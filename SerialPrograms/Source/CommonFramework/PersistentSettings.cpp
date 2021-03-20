/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QCoreApplication>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "Common/Qt/StringException.h"
#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch/Framework/VirtualSwitchControllerMapping.h"
#include "PanelList.h"
#include "PersistentSettings.h"

namespace PokemonAutomation{




PersistentSettings settings;


void PersistentSettings::write() const{
    QJsonObject root;
    root.insert("00-ConfigPath", QJsonValue(config_path));
    root.insert("01-SourcePath", QJsonValue(source_path));
    {
        QJsonArray res;
        res.append(QJsonValue(window_size.width()));
        res.append(QJsonValue(window_size.height()));
        root.insert("02-WindowSize", res);
    }
    root.insert("03-NaughtyMode", QJsonValue(naughty_mode));
    root.insert("04-DeveloperMode", QJsonValue(developer_mode));
    root.insert("05-LogEverything", QJsonValue(log_everything));

    root.insert("10-SwitchKeyboardMapping", NintendoSwitch::read_keyboard_mapping());

    QJsonObject settings;
    for (const auto& panel : SETTINGS_MAP()){
        settings.insert(panel.first, panel.second->to_json());
    }
    root.insert("98-SharedSettings", settings);

    QJsonObject programs;
    for (const auto& panel : PROGRAM_MAP()){
        programs.insert(panel.first, panel.second->to_json());
    }
    root.insert("99-ProgramSettings", programs);

    write_json_file(QCoreApplication::applicationFilePath() + "-Settings.json", QJsonDocument(root));
}
void PersistentSettings::read(){
    QJsonDocument doc = read_json_file(QCoreApplication::applicationFilePath() + "-Settings.json");
    if (!doc.isObject()){
        throw StringException("Invalid settings file.");
    }
    QJsonObject root = doc.object();
    config_path = json_get_string(root, "00-ConfigPath");
    source_path = json_get_string(root, "01-SourcePath");
    {
        QJsonArray res = json_get_array(root, "02-WindowSize");
        if (res.size() == 2){
            window_size = QSize(
                res[0].toInt(window_size.width()),
                res[1].toInt(window_size.height())
            );
        }
    }
    naughty_mode = json_get_bool(root, "03-NaughtyMode");
    developer_mode = json_get_bool(root, "04-DeveloperMode");
    log_everything = json_get_bool(root, "05-LogEverything");
    NintendoSwitch::set_keyboard_mapping(json_get_array(root, "10-SwitchKeyboardMapping"));
    settings = json_get_object(root, "98-SharedSettings");
    programs = json_get_object(root, "99-ProgramSettings");
}



}

