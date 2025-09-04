/*  Multi-Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemOption::MultiSwitchSystemOption(
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    size_t min_switches,
    size_t max_switches,
    size_t switches
)
    : m_allow_commands_while_running(allow_commands_while_running == AllowCommandsWhenRunning::ENABLE_COMMANDS)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    switches = std::max(switches, m_min_switches);
    switches = std::min(switches, m_max_switches);
    resize(switches);
}
MultiSwitchSystemOption::MultiSwitchSystemOption(
    FeedbackType feedback,
    AllowCommandsWhenRunning allow_commands_while_running,
    size_t min_switches,
    size_t max_switches,
    const JsonValue& json
)
    : m_allow_commands_while_running(allow_commands_while_running == AllowCommandsWhenRunning::ENABLE_COMMANDS)
    , m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    MultiSwitchSystemOption::load_json(json);
    if (m_switches.size() < m_min_switches){
        resize(m_min_switches);
    }
}
void MultiSwitchSystemOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    const JsonArray* array = obj->get_array("DeviceList");
    if (array != nullptr && !array->empty() && array->size() <= MAX_SWITCHES){
        m_switches.clear();
        size_t items = array->size();
        for (size_t c = 0; c < items; c++){
            m_switches.emplace_back(
                new SwitchSystemOption(
                    m_allow_commands_while_running,
                    (*array)[c]
                )
            );
        }
    }
    obj->read_integer(m_active_switches, "ActiveDevices", m_min_switches, m_max_switches);
}
JsonValue MultiSwitchSystemOption::to_json() const{
    JsonObject obj;
    obj["ActiveDevices"] = m_active_switches;
    JsonArray array;
    for (const auto& item : m_switches){
        array.push_back(item->to_json());
    }
    obj["DeviceList"] = std::move(array);
    return obj;
}
void MultiSwitchSystemOption::resize(size_t count){
    while (m_switches.size() < count){
        m_switches.emplace_back(
            new SwitchSystemOption(m_allow_commands_while_running)
        );
    }
    m_active_switches = count;
}





}
}





