/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CameraOption.h"

namespace PokemonAutomation{


const std::string CameraOption::JSON_CAMERA       = "Device";
const std::string CameraOption::JSON_RESOLUTION   = "Resolution";


CameraOption::CameraOption(Resolution p_default_resolution)
    : default_resolution(p_default_resolution)
    , current_resolution(p_default_resolution)
{}

void CameraOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const std::string* name = obj->get_string(JSON_CAMERA);
    if (name != nullptr){
        info = CameraInfo(*name);
    }
    const JsonArray* res = obj->get_array(JSON_RESOLUTION);
    if (res != nullptr && res->size() == 2){
        do{
            size_t width, height;
            if (!(*res)[0].read_integer(width)){
                break;
            }
            if (!(*res)[1].read_integer(height)){
                break;
            }
            current_resolution = Resolution(width, height);
        }while (false);
    }
}
JsonValue CameraOption::to_json() const{
    JsonObject root;
    root[JSON_CAMERA] = info.device_name();
    JsonArray res;
    res.push_back(current_resolution.width);
    res.push_back(current_resolution.height);
    root[JSON_RESOLUTION] = std::move(res);
    return root;
}











}
