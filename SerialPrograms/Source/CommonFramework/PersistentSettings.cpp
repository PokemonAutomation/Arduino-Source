/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <set>
#include <QCoreApplication>
#include <QFile>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include "Common/Cpp/Exceptions.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/LoggerQt.h"
#include "NintendoSwitch/Framework/NintendoSwitch_VirtualControllerMapping.h"
#include "PersistentSettings.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{



PersistentSettings& PERSISTENT_SETTINGS(){
    static PersistentSettings settings;
    return settings;
}


PersistentSettings::PersistentSettings(){}



void PersistentSettings::write() const{
    QJsonObject root;

    root.insert("20-GlobalSettings", GlobalSettings::instance().to_json());
    root.insert("50-SwitchKeyboardMapping", NintendoSwitch::read_keyboard_mapping());

    root.insert("99-Panels", panels);
//    cout << QJsonDocument(panels).toJson().data() << endl;

    try{
        write_json_file(QCoreApplication::applicationFilePath() + "-Settings.json", QJsonDocument(root));
    }catch (FileException&){
    }
}
void PersistentSettings::read(){
    QString path = QCoreApplication::applicationFilePath() + "-Settings.json";
    QJsonDocument doc = read_json_file(path);
    if (!doc.isObject()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid settings file.", path.toStdString());
    }
    QJsonObject root = doc.object();

    GlobalSettings::instance().load_json(json_get_object_nothrow(root,"20-GlobalSettings"));
//    GlobalSettings::instance().PROCESS_PRIORITY0.update_priority_to_option();
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY.set_on_this_thread();

    NintendoSwitch::set_keyboard_mapping(json_get_array_nothrow(root, "50-SwitchKeyboardMapping"));

    panels = json_get_object_nothrow(root, "99-Panels");
}



}

