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
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "NintendoSwitch/Framework/VirtualSwitchControllerMapping.h"
#include "CommonFramework/GlobalSettingsPanel.h"
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

    write_json_file(QCoreApplication::applicationFilePath() + "-Settings.json", QJsonDocument(root));
}
void PersistentSettings::read(){
    QJsonDocument doc = read_json_file(QCoreApplication::applicationFilePath() + "-Settings.json");
    if (!doc.isObject()){
        PA_THROW_ParseException("Invalid settings file.");
    }
    QJsonObject root = doc.object();

    GlobalSettings::instance().load_json(json_get_object_nothrow(root,"20-GlobalSettings"));
    NintendoSwitch::set_keyboard_mapping(json_get_array_nothrow(root, "50-SwitchKeyboardMapping"));

    panels = json_get_object_nothrow(root, "99-Panels");
}



}

