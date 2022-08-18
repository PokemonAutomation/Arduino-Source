/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <map>
#include <atomic>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "EnumDropdownOption.h"

namespace PokemonAutomation{


struct DropdownCell::Data{
    std::vector<DropdownEntry> m_case_list;
    std::map<std::string, size_t> m_slug_to_index;
    std::map<std::string, size_t> m_display_to_index;
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
            const DropdownEntry& item = m_case_list.back();
            auto ret = m_slug_to_index.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(item.slug),
                std::forward_as_tuple(index)
            );
            if (!ret.second){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum Slug: " + item.slug);
            }
            ret = m_display_to_index.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(item.display_name),
                std::forward_as_tuple(index)
            );
            if (!ret.second){
                throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Enum Name: " + item.display_name);
            }
        }
    }
    Data(std::vector<DropdownEntry> cases, size_t default_index)
        : m_default(default_index)
        , m_current(default_index)
    {
        if (default_index >= cases.size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
        }
        for (size_t index = 0; index < cases.size(); index++){
            const std::string& item = cases[index].slug;
            auto ret = m_slug_to_index.emplace(
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
    Data(std::initializer_list<DropdownEntry> cases, size_t default_index)
        : m_default(default_index)
        , m_current(default_index)
    {
        if (default_index >= cases.size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Index is too large: " + std::to_string(default_index));
        }
        size_t index = 0;
        for (auto iter = cases.begin(); iter != cases.end(); ++iter){
            m_case_list.emplace_back(*iter);
            const std::string& item = m_case_list.back().slug;
            auto ret = m_slug_to_index.emplace(
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




DropdownCell::~DropdownCell() = default;
DropdownCell::DropdownCell(const DropdownCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.default_index())
{
    m_data->m_case_list = x.m_data->m_case_list;
    m_data->m_slug_to_index = x.m_data->m_slug_to_index;
    m_data->m_current.store(x.m_data->m_current.load(std::memory_order_relaxed), std::memory_order_relaxed);
}


DropdownCell::DropdownCell(
    std::vector<std::string> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}
DropdownCell::DropdownCell(
    std::vector<DropdownEntry> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}
DropdownCell::DropdownCell(
    std::initializer_list<DropdownEntry> cases,
    size_t default_index
)
    : m_data(CONSTRUCT_TOKEN, std::move(cases), default_index)
{}


const std::vector<DropdownEntry>& DropdownCell::case_list() const{
    return m_data->m_case_list;
}
size_t DropdownCell::default_index() const{
    return m_data->m_default;
}
size_t DropdownCell::current_index() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
const std::string& DropdownCell::case_name(size_t index) const{
    return m_data->m_case_list[index].slug;
}
const std::string& DropdownCell::current_case() const{
    const Data& data = *m_data;
    return data.m_case_list[data.m_current].slug;
}

#if 1
DropdownCell::operator size_t() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
#endif

bool DropdownCell::set_index(size_t index){
    Data& data = *m_data;
    if (index >= data.m_case_list.size()){
        return false;
    }
    data.m_current.store(index, std::memory_order_relaxed);
    push_update();
    return true;
}

void DropdownCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    Data& data = *m_data;
    auto iter = data.m_slug_to_index.find(*str);
    if (iter != data.m_slug_to_index.end() && data.m_case_list[iter->second].enabled){
        data.m_current.store(iter->second, std::memory_order_relaxed);
        push_update();
    }

    //  Backward compatibility with display names.
    iter = data.m_display_to_index.find(*str);
    if (iter != data.m_display_to_index.end() && data.m_case_list[iter->second].enabled){
        data.m_current.store(iter->second, std::memory_order_relaxed);
        push_update();
    }
}
JsonValue DropdownCell::to_json() const{
    const Data& data = *m_data;
    return data.m_case_list[data.m_current].slug;
}

void DropdownCell::restore_defaults(){
    Data& data = *m_data;
    data.m_current.store(data.m_default, std::memory_order_relaxed);
    push_update();
}







}
