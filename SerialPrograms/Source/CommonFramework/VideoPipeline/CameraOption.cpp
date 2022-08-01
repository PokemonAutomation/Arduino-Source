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
#include "CameraSelectorWidget.h"

namespace PokemonAutomation{


const std::string CameraOption::JSON_CAMERA       = "Device";
const std::string CameraOption::JSON_RESOLUTION   = "Resolution";


CameraOption::CameraOption(Resolution default_resolution)
    : m_default_resolution(default_resolution)
    , m_current_resolution(default_resolution)
{}
CameraOption::CameraOption(Resolution default_resolution, const JsonValue& json)
    : CameraOption(default_resolution)
{
    load_json(json);
}

void CameraOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* name = obj->get_string(JSON_CAMERA);
    if (name != nullptr){
        m_camera = CameraInfo(*name);
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
            m_current_resolution = Resolution(width, height);
        }while (false);
    }
}
JsonValue CameraOption::to_json() const{
    JsonObject root;
    root[JSON_CAMERA] = m_camera.device_name();
    JsonArray res;
    res.push_back(m_current_resolution.width);
    res.push_back(m_current_resolution.height);
    root[JSON_RESOLUTION] = std::move(res);
    return root;
}

CameraSelectorWidget* CameraOption::make_ui(QWidget& parent, LoggerQt& logger, VideoDisplayWidget& holder){
    return new CameraSelectorWidget(parent, logger, *this, holder);
}











}
