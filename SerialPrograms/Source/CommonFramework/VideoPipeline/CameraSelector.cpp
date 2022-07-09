/*  Camera Selector
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Compiler.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "CameraSelector.h"
#include "CameraSelectorWidget.h"

namespace PokemonAutomation{


const std::string CameraSelector::JSON_CAMERA       = "Device";
const std::string CameraSelector::JSON_RESOLUTION   = "Resolution";


CameraSelector::CameraSelector(QSize default_resolution)
    : m_default_resolution(default_resolution)
    , m_current_resolution(default_resolution)
{}
CameraSelector::CameraSelector(QSize default_resolution, const JsonValue2& json)
    : CameraSelector(default_resolution)
{
    load_json(json);
}

void CameraSelector::load_json(const JsonValue2& json){
    const JsonObject2* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    const std::string* name = obj->get_string(JSON_CAMERA);
    if (name != nullptr){
        m_camera = CameraInfo(*name);
    }
    const JsonArray2* res = obj->get_array(JSON_RESOLUTION);
    if (res != nullptr && res->size() == 2){
        do{
            int width, height;
            if (!(*res)[0].read_integer(width)){
                break;
            }
            if (!(*res)[1].read_integer(height)){
                break;
            }
            m_current_resolution = QSize(width, height);
        }while (false);
    }
}
JsonValue2 CameraSelector::to_json() const{
    JsonObject2 root;
    root[JSON_CAMERA] = m_camera.device_name();
    JsonArray2 res;
    res.push_back(m_current_resolution.width());
    res.push_back(m_current_resolution.height());
    root[JSON_RESOLUTION] = std::move(res);
    return root;
}

CameraSelectorWidget* CameraSelector::make_ui(QWidget& parent, LoggerQt& logger, VideoDisplayWidget& holder){
    return new CameraSelectorWidget(parent, logger, *this, holder);
}











}
