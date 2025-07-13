/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "ML_ImageDisplayOption.h"

namespace PokemonAutomation{
namespace ML{


const std::string ImageDisplayOption::JSON_VIDEO        = "Video";
const std::string ImageDisplayOption::JSON_OVERLAY      = "Overlay";


ImageDisplayOption::ImageDisplayOption()
{}
ImageDisplayOption::ImageDisplayOption(const JsonValue& json)
{
    load_json(json);
}
void ImageDisplayOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
//    json_get_bool(m_settings_visible, obj, "SettingsVisible");
    const JsonValue* value = nullptr;
    value = obj->get_value(JSON_VIDEO);
    if (value){
        m_video.load_json(*value);
    }
    value = obj->get_value(JSON_OVERLAY);
    if (value){
        m_overlay.load_json(*value);
    }
}
JsonValue ImageDisplayOption::to_json() const{
    JsonObject root;
    root[JSON_VIDEO] = m_video.to_json();
    root[JSON_OVERLAY] = m_overlay.to_json();

    return root;
}







}
}

