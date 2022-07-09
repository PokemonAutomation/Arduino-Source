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
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Json/JsonTools.h"
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
    JsonObject root;

    root["20-GlobalSettings"] = GlobalSettings::instance().to_json();
    root["50-SwitchKeyboardMapping"] = NintendoSwitch::read_keyboard_mapping();

    root["99-Panels"] = from_QJson(panels);
//    cout << QJsonDocument(panels).toJson().data() << endl;

    try{
        root.dump(QCoreApplication::applicationName().toStdString() + "-Settings.json");
    }catch (FileException&){
    }
}


void PersistentSettings::read(){
    QString path = QCoreApplication::applicationName() + "-Settings.json";
    QJsonDocument doc = read_json_file(path);
    if (!doc.isObject()){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid settings file.", path.toStdString());
    }
    JsonValue root = from_QJson(doc.object());
    const JsonObject* obj = root.get_object();

    //  Need to load this subset of settings first because they will affect how
    //  "GlobalSettings" is constructed.
    const JsonValue* settings = obj->get_value("20-GlobalSettings");
    if (settings){
        PreloadSettings::instance().load(*settings);
        GlobalSettings::instance().load_json(*settings);
    }

//    GlobalSettings::instance().PROCESS_PRIORITY0.update_priority_to_option();
    GlobalSettings::instance().REALTIME_THREAD_PRIORITY0.set_on_this_thread();

    {
        const JsonArray* array = obj->get_array("50-SwitchKeyboardMapping");
        if (array){
            NintendoSwitch::set_keyboard_mapping(*array);
        }
    }
    {
        const JsonValue* value = obj->get_value("99-Panels");
        if (value){
            panels = to_QJson(*value).toObject();
        }
    }
}



}

