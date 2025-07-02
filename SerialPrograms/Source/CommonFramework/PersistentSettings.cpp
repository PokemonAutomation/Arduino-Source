/*  Persistent Settings
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QCoreApplication>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
//#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CommonFramework/Globals.h"
#include "CommonFramework/GlobalSettingsPanel.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonFramework/Options/Environment/PerformanceOptions.h"
#include "PersistentSettings.h"

// #include <iostream>
// using std::cout;
// using std::endl;

namespace PokemonAutomation{



PersistentSettings& PERSISTENT_SETTINGS(){
    static PersistentSettings settings;
    return settings;
}


PersistentSettings::PersistentSettings(){}



void PersistentSettings::write() const{
    JsonObject root;

    root["20-GlobalSettings"] = GlobalSettings::instance().to_json();
//    root["50-SwitchKeyboardMapping"] = NintendoSwitch::read_keyboard_mapping();

    root["99-Panels"] = panels.clone();

    try{
        root.dump(PROGRAM_SETTING_JSON_PATH());
    }catch (FileException&){}
}


void PersistentSettings::read(){
    JsonValue json = load_json_file(PROGRAM_SETTING_JSON_PATH());
    JsonObject* obj = json.to_object();
    if (obj == nullptr){
        throw FileException(nullptr, PA_CURRENT_FUNCTION, "Invalid settings file.", PROGRAM_SETTING_JSON_PATH());
    }

    //  Need to load this subset of settings first because they will affect how
    //  "GlobalSettings" is constructed.
    const JsonValue* settings = obj->get_value("20-GlobalSettings");
    if (settings){
        PreloadSettings::instance().load(*settings);
        GlobalSettings::instance().load_json(*settings);
    }

//    GlobalSettings::instance().PROCESS_PRIORITY0.update_priority_to_option();
    GlobalSettings::instance().PERFORMANCE->REALTIME_THREAD_PRIORITY.set_on_this_thread(global_logger_tagged());

#if 0
    {
        const JsonArray* array = obj->get_array("50-SwitchKeyboardMapping");
        if (array){
            NintendoSwitch::set_keyboard_mapping(*array);
        }
    }
#endif
    {
        JsonObject* value = obj->get_object("99-Panels");
        if (value){
//            panels = to_QJson(*value).toObject();
            panels = std::move(*value);
        }
    }
}



}

