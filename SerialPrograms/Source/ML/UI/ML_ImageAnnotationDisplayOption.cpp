/*  Switch System
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch/NintendoSwitch_Settings.h"
#include "ML_ImageAnnotationDisplayOption.h"

namespace PokemonAutomation{
namespace ML{

static const std::string JSON_IMAGE_PATH = "ImagePath";
static const std::string JSON_OVERLAY = "Overlay";


ImageAnnotationDisplayOption::ImageAnnotationDisplayOption()
{}
ImageAnnotationDisplayOption::ImageAnnotationDisplayOption(const JsonValue& json)
{
    load_json(json);
}
void ImageAnnotationDisplayOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }

    const std::string* json_str = obj->get_string(JSON_IMAGE_PATH);
    if (json_str){
        m_image_path = *json_str;
    }
    const JsonValue* value = obj->get_value(JSON_OVERLAY);
    if (value){
        m_overlay.load_json(*value);
    }
}
JsonValue ImageAnnotationDisplayOption::to_json() const{
    JsonObject root;
    root[JSON_IMAGE_PATH] = m_image_path;
    root[JSON_OVERLAY] = m_overlay.to_json();

    return root;
}







}
}

