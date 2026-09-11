/*  Multi-Switch System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "NintendoSwitch_MultiSwitchSystemOption.h"

namespace PokemonAutomation{
namespace NintendoSwitch{


MultiSwitchSystemOption::MultiSwitchSystemOption(
    size_t min_switches,
    size_t max_switches,
    size_t switches
)
    : m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
    , m_switches(m_max_switches)
{
    for (size_t c = 0; c < m_max_switches; c++){
        m_switches.emplace_back();
    }
    switches = std::max(switches, m_min_switches);
    switches = std::min(switches, m_max_switches);
    m_active_switches = switches;
}
MultiSwitchSystemOption::MultiSwitchSystemOption(
    size_t min_switches,
    size_t max_switches,
    const JsonValue& json
)
    : m_min_switches(std::max(min_switches, (size_t)1))
    , m_max_switches(std::min(max_switches, (size_t)MAX_SWITCHES))
    , m_active_switches(0)
{
    for (size_t c = 0; c < m_max_switches; c++){
        m_switches.emplace_back();
    }
    MultiSwitchSystemOption::load_json(json);
}
void MultiSwitchSystemOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    obj->read_integer(m_active_switches, "ActiveDevices", m_min_switches, m_max_switches);
    const JsonArray* array = obj->get_array("DeviceList");
    if (array != nullptr){
        size_t c = 0;
        size_t stop = std::min(m_max_switches, array->size());
        for (; c < stop; c++){
            m_switches[c].load_json((*array)[c]);
        }
        m_active_switches = c;
    }
}
JsonValue MultiSwitchSystemOption::to_json() const{
    JsonObject obj;
    obj["ActiveDevices"] = m_active_switches;
    JsonArray array;
    for (const auto& item : m_switches){
        array.push_back(item.to_json());
    }
    obj["DeviceList"] = std::move(array);
    return obj;
}
void MultiSwitchSystemOption::resize(size_t count){
    m_active_switches = count;
}





}
}





