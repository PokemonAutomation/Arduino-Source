/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "EnumDropdownOption.h"

namespace PokemonAutomation{


EnumDropdownOption::EnumDropdownOption(
    std::string label,
    std::initializer_list<Option> cases,
    size_t default_index
)
    : m_label(std::move(label))
    , m_default(default_index)
    , m_current(default_index)
{
    if (default_index >= cases.size()){
        throw "Index is too large.";
    }
    size_t index = 0;
    for (auto iter = cases.begin(); iter != cases.end(); ++iter){
        m_case_list.emplace_back(*iter);
        const std::string& item = m_case_list.back().name;
        auto ret = m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(item),
            std::forward_as_tuple(index++)
        );
        if (!ret.second){
            throw "Duplicate enum label.";
        }
    }
}

void EnumDropdownOption::set(size_t index){
    m_current.store(index, std::memory_order_relaxed);
    push_update();
}

void EnumDropdownOption::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    auto iter = m_case_map.find(*str);
    if (iter != m_case_map.end() && m_case_list[iter->second].enabled){
        m_current.store(iter->second, std::memory_order_relaxed);
        push_update();
    }
}
JsonValue EnumDropdownOption::to_json() const{
    return m_case_list[m_current].name;
}

void EnumDropdownOption::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    push_update();
}



}
