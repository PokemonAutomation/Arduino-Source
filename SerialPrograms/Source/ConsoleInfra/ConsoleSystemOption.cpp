/*  Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Controllers/NullController.h"
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
    bool allow_commands_while_locked
)
    : m_allow_commands_while_locked(allow_commands_while_locked)
    , m_controllers(num_controllers)
{
    if (num_controllers == 0){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "num_controllers cannot be 0.");
    }
    bool enable_input = true;
    for (size_t c = 0; c < num_controllers; c++){
        m_controllers.emplace_back(enable_input);
        enable_input = false;
    }
}
ConsoleSystemOption::ConsoleSystemOption(
    size_t num_controllers,
    bool allow_commands_while_locked,
    const JsonValue& json
)
    : ConsoleSystemOption(num_controllers, allow_commands_while_locked)
{
    ConsoleSystemOption::load_json(json);
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
        m_controllers[0].load_json(*value);
    }
    const JsonArray* array = obj->get_array(JSON_CONTROLLERS);
    if (array){
        size_t c = 0;
        size_t stop = std::min(m_controllers.size(), array->size());
        for (; c < stop; c++){
            m_controllers[c].load_json((*array)[c]);
        }
        stop = m_controllers.size();
        for (; c < stop; c++){
            m_controllers[c].set_descriptor(std::make_shared<NullControllerDescriptor>());
        }
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
