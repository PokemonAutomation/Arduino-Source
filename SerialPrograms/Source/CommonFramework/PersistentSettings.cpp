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
#include "PanelList.h"
#include "PersistentSettings.h"

namespace PokemonAutomation{




PersistentSettings settings;


void PersistentSettings::write() const{
    QJsonObject root;
    root.insert("ConfigPath", QJsonValue(config_path));
    root.insert("SourcePath", QJsonValue(source_path));
    {
        QJsonArray res;
        res.append(QJsonValue(window_size.width()));
        res.append(QJsonValue(window_size.height()));
        root.insert("WindowSize", res);
    }
    root.insert("NaughtyMode", QJsonValue(naughty_mode));
    root.insert("DeveloperMode", QJsonValue(developer_mode));
    root.insert("LogEverything", QJsonValue(log_everything));

    QJsonObject settings;
    for (const auto& panel : SETTINGS_MAP()){
        settings.insert(panel.first, panel.second->to_json());
    }
    root.insert("Settings", settings);

    QJsonObject programs;
    for (const auto& panel : PROGRAM_MAP()){
        programs.insert(panel.first, panel.second->to_json());
    }
    root.insert("Programs", programs);

    write_json_file(QCoreApplication::applicationFilePath() + "-Settings.json", QJsonDocument(root));
}
void PersistentSettings::read(){
    QJsonDocument doc = read_json_file(QCoreApplication::applicationFilePath() + "-Settings.json");
    if (!doc.isObject()){
        throw StringException("Invalid settings file.");
    }
    QJsonObject root = doc.object();
    config_path = json_get_string(root, "ConfigPath");
    source_path = json_get_string(root, "SourcePath");
    {
        QJsonArray res = json_get_array(root, "WindowSize");
        if (res.size() == 2){
            window_size = QSize(
                res[0].toInt(window_size.width()),
                res[1].toInt(window_size.height())
            );
        }
    }
    naughty_mode = json_get_bool(root, "NaughtyMode");
    developer_mode = json_get_bool(root, "DeveloperMode");
    log_everything = json_get_bool(root, "LogEverything");
    settings = json_get_object(root, "Settings");
    programs = json_get_object(root, "Programs");
}



}

