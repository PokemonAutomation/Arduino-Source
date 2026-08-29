/*  Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "ConsoleInfra/ConsoleSystemOption.h"

namespace PokemonAutomation{
namespace ConsoleInfra{

const std::string ConsoleSystemOption::JSON_CAMERA      = "Camera";
const std::string ConsoleSystemOption::JSON_VIDEO       = "Video";
const std::string ConsoleSystemOption::JSON_AUDIO       = "Audio";
const std::string ConsoleSystemOption::JSON_OVERLAY     = "Overlay";
const std::string ConsoleSystemOption::JSON_CONTROLLER  = "Controller";
const std::string ConsoleSystemOption::JSON_CONTROLLERS = "Controllers";



ConsoleSystemOption::ConsoleSystemOption(
    size_t num_controllers,
    bool allow_commands_while_running
)
    : m_allow_commands_while_running(allow_commands_while_running)
    , m_controllers(num_controllers)
{}
ConsoleSystemOption::ConsoleSystemOption(
    size_t num_controllers,
    bool allow_commands_while_running,
    const JsonValue& json
)
    : ConsoleSystemOption(num_controllers, allow_commands_while_running)
{
    load_json(json);
}
void ConsoleSystemOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonValue* value;
    value = obj->get_value(JSON_VIDEO);
    if (value){
        m_video.load_json(*value);
    }
    value = obj->get_value(JSON_AUDIO);
    if (value){
        m_audio.load_json(*value);
    }
    value = obj->get_value(JSON_OVERLAY);
    if (value){
        m_overlay.load_json(*value);
    }
    value = obj->get_value(JSON_CONTROLLER);
    if (value){
        std::vector<ControllerOption> controllers;
        controllers.emplace_back().load_json(*value);
        m_controllers = std::move(controllers);
    }
    value = obj->get_value(JSON_CONTROLLERS);
    if (value){
        std::vector<ControllerOption> controllers;
        if (value->is_array()){
            for (const JsonValue& item : *value->to_array()){
                controllers.emplace_back().load_json(item);
            }
        }else{
            controllers.emplace_back().load_json(*value);
        }
        m_controllers = std::move(controllers);
    }
}
JsonValue ConsoleSystemOption::to_json() const{
    JsonObject root;
    root[JSON_VIDEO] = m_video.to_json();
    root[JSON_AUDIO] = m_audio.to_json();
    root[JSON_OVERLAY] = m_overlay.to_json();
    if (m_controllers.size() == 1){
        root[JSON_CONTROLLER] = m_controllers[0].to_json();
    }else{
        JsonArray list;
        for (const ControllerOption& controller : m_controllers){
            list.push_back(controller.to_json());
        }
        root[JSON_CONTROLLERS] = std::move(list);
    }

    return root;
}




}
}
