/*  Path Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "PathOption.h"

namespace PokemonAutomation{



struct PathCell::Data{
    const std::string m_default;
    const std::string m_filter_string;
    const std::string m_placeholder_text;

    std::atomic<bool> m_locked;

    mutable SpinLock m_lock;
    std::string m_current;

    Data(
        std::string default_value,
        std::string filter_string,
        std::string placeholder_text
    )
        : m_default(std::move(default_value))
        , m_filter_string(std::move(filter_string))
        , m_placeholder_text(std::move(placeholder_text))
        , m_current(m_default)
    {}
};


PathCell::~PathCell() = default;
PathCell::PathCell(
    LockMode lock_while_program_is_running,
    std::string default_value,
    std::string filter_string,
    std::string placeholder_text
)
    : ConfigOptionImpl<PathCell>(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, std::move(default_value), std::move(filter_string), std::move(placeholder_text))
{}

const std::string& PathCell::placeholder_text() const{
    return m_data->m_placeholder_text;
}
const std::string PathCell::default_value() const{
    return m_data->m_default;
}
const std::string& PathCell::filter_string() const{
    return m_data->m_filter_string;
}


bool PathCell::is_locked() const{
    return m_data->m_locked.load(std::memory_order_relaxed);
}
void PathCell::set_locked(bool locked){
    if (locked == is_locked()){
        return;
    }
    m_data->m_locked.store(locked, std::memory_order_relaxed);
    report_visibility_changed();
}


PathCell::operator std::string() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
void PathCell::set(std::string x){
    sanitize(x);
    {
        WriteSpinLock lg(m_data->m_lock);
        if (m_data->m_current == x){
            return;
        }
        m_data->m_current = std::move(x);
    }
    report_value_changed(this);
}

void PathCell::load_json(const JsonValue& json){
    const std::string* str = json.to_string();
    if (str == nullptr){
        return;
    }
    set(*str);
}
JsonValue PathCell::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}

void PathCell::restore_defaults(){
    set(m_data->m_default);
}



PathOption::PathOption(
    std::string label,
    LockMode lock_while_program_is_running,
    std::string default_value,
    std::string filter_string,
    std::string placeholder_text
)
     : ConfigOptionImpl<PathOption, PathCell>(
        lock_while_program_is_running,
        default_value,
        filter_string,
        placeholder_text
    )
     , m_label(std::move(label))
{}







}
