/*  Multi-Console System Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "MultiConsoleSystemOption.h"

namespace PokemonAutomation{
namespace GameConsole{



MultiConsoleSystemOption::~MultiConsoleSystemOption(){

}
MultiConsoleSystemOption::MultiConsoleSystemOption(
    size_t min_consoles,
    size_t max_consoles,
    size_t consoles,
    std::function<std::unique_ptr<ConsoleSystemOption>()> console_factory
)
    : m_min_consoles(std::max(min_consoles, (size_t)1))
    , m_max_consoles(std::min(max_consoles, (size_t)MAX_CONSOLES))
    , m_active_consoles(0)
    , m_consoles(m_max_consoles)
{
    for (size_t c = 0; c < m_max_consoles; c++){
        m_consoles.emplace_back(console_factory());
    }
    consoles = std::max(consoles, m_min_consoles);
    consoles = std::min(consoles, m_max_consoles);
    m_active_consoles = consoles;
}
MultiConsoleSystemOption::MultiConsoleSystemOption(
    size_t min_consoles,
    size_t max_consoles,
    const JsonValue& json,
    std::function<std::unique_ptr<ConsoleSystemOption>()> console_factory
)
    : MultiConsoleSystemOption(min_consoles, max_consoles, 0, std::move(console_factory))
{
    MultiConsoleSystemOption::load_json(json);
}

JsonValue MultiConsoleSystemOption::to_json() const{
    JsonObject obj;
    obj["ActiveDevices"] = m_active_consoles;
    {
        JsonArray array;
        for (const auto& item : m_consoles){
            array.push_back(item->to_json());
        }
        obj["DeviceList"] = std::move(array);
    }
    return obj;
}
void MultiConsoleSystemOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    obj->read_integer(
        m_active_consoles,
        "ActiveDevices",
        m_min_consoles,
        m_max_consoles
    );
    {
        const JsonArray* array = obj->get_array("DeviceList");
        if (array != nullptr){
            size_t c = 0;
            size_t stop = std::min(m_max_consoles, array->size());
            for (; c < stop; c++){
                m_consoles[c]->load_json((*array)[c]);
            }
            m_active_consoles = c;
        }
    }
}




}
}
