/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <atomic>
#include "Common/Cpp/Pimpl.tpp"
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "EnumDropdownOption.h"

namespace PokemonAutomation{


struct EnumDropdownCell::Data{
    std::vector<EnumDropdownEntry> m_case_list;
    std::map<std::string, size_t> m_case_map;
    const size_t m_default;
    std::atomic<size_t> m_current;

    Data(size_t default_index)
        : m_default(default_index)
    {}
    Data(std::vector<std::string> cases, size_t default_index)
        : m_default(default_index)
        , m_current(default_index)
    {
        if (default_index >= cases.size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
        }
        for (size_t index = 0; index < cases.size(); index++){
            m_case_list.emplace_back(std::move(cases[index]));
            const std::string& item = m_case_list.back().name;
            auto ret = m_case_map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(item),
                std::forward_as_tuple(index)
            );
            if (!ret.second){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label: " + item);
            }
        }
    }
    Data(std::vector<EnumDropdownEntry> cases, size_t default_index)
        : m_default(default_index)
        , m_current(default_index)
    {
        if (default_index >= cases.size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
        }
        for (size_t index = 0; index < cases.size(); index++){
            const std::string& item = cases[index].name;
            auto ret = m_case_map.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(item),
                std::forward_as_tuple(index)
            );
            if (!ret.second){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label: " + item);
            }
        }
        m_case_list = std::move(cases);
    }
    Data(std::initializer_list<EnumDropdownEntry> cases, size_t default_index)
        : m_default(default_index)
        , m_current(default_index)
    {
        if (default_index >= cases.size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
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
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate enum label: " + item);
            }
        }
    }
};




EnumDropdownCell::~EnumDropdownCell() = default;
EnumDropdownCell::EnumDropdownCell(const EnumDropdownCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.default_index())
{
    m_data->m_case_list = x.m_data->m_case_list;
    m_data->m_case_map = x.m_data->m_case_map;
    m_data->m_current.store(x.m_data->m_current.load(std::memory_order_relaxed), std::memory_order_relaxed);
}


EnumDropdownCell::EnumDropdownCell(
    std::vector<std::string> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}
EnumDropdownCell::EnumDropdownCell(
    std::vector<EnumDropdownEntry> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}
EnumDropdownCell::EnumDropdownCell(
    std::initializer_list<EnumDropdownEntry> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}


const std::vector<EnumDropdownEntry>& EnumDropdownCell::case_list() const{
    return m_data->m_case_list;
}
size_t EnumDropdownCell::default_index() const{
    return m_data->m_default;
}
const std::string& EnumDropdownCell::case_name(size_t index) const{
    return m_data->m_case_list[index].name;
}
const std::string& EnumDropdownCell::current_case() const{
    const Data& data = *m_data;
    return data.m_case_list[data.m_current].name;
}

EnumDropdownCell::operator size_t() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}

bool EnumDropdownCell::set(size_t index){
    Data& data = *m_data;
    if (index >= data.m_case_list.size()){
        return false;
    }
    data.m_current.store(index, std::memory_order_relaxed);
    push_update();
    return true;
}

void EnumDropdownCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    Data& data = *m_data;
    auto iter = data.m_case_map.find(*str);
    if (iter != data.m_case_map.end() && data.m_case_list[iter->second].enabled){
        data.m_current.store(iter->second, std::memory_order_relaxed);
        push_update();
    }
}
JsonValue EnumDropdownCell::to_json() const{
    const Data& data = *m_data;
    return data.m_case_list[data.m_current].name;
}

void EnumDropdownCell::restore_defaults(){
    Data& data = *m_data;
    data.m_current.store(data.m_default, std::memory_order_relaxed);
    push_update();
}







}
