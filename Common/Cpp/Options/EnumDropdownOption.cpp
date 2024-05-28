/*  Enum Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <atomic>
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "EnumDropdownOption.h"

namespace PokemonAutomation{


struct IntegerEnumDropdownCell::Data{
    const IntegerEnumDatabase& m_database;
    const size_t m_default;
    std::atomic<size_t> m_current;

    Data(const IntegerEnumDatabase& database, size_t default_value, size_t current_value)
        : m_database(database)
        , m_default(default_value)
        , m_current(current_value)
    {}
};




IntegerEnumDropdownCell::~IntegerEnumDropdownCell() = default;
IntegerEnumDropdownCell::IntegerEnumDropdownCell(const IntegerEnumDropdownCell& x)
    : ConfigOption(x)
    , m_data(CONSTRUCT_TOKEN, x.database(), x.default_value(), x.current_value())
{}
IntegerEnumDropdownCell::IntegerEnumDropdownCell(
    const IntegerEnumDatabase& database,
    LockMode lock_while_running,
    size_t default_value, size_t current_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, database, default_value, current_value)
{
    if (database.find(default_value) == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Default value is not in the database: " + std::to_string(default_value));
    }
    if (database.find(current_value) == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Current value is not in the database: " + std::to_string(current_value));
    }
}
IntegerEnumDropdownCell::IntegerEnumDropdownCell(
    const IntegerEnumDatabase& database,
    LockMode lock_while_running,
    size_t default_value
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, database, default_value, default_value)
{
    if (database.find(default_value) == nullptr){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Default value is not in the database: " + std::to_string(default_value));
    }
}
size_t IntegerEnumDropdownCell::default_value() const{
    return m_data->m_default;
}
size_t IntegerEnumDropdownCell::current_value() const{
    return m_data->m_current.load(std::memory_order_relaxed);
}
bool IntegerEnumDropdownCell::set_value(size_t value){
    Data& data = *m_data;
    if (data.m_database.find(value) == nullptr){
        return false;
    }
    if (value != data.m_current.exchange(value, std::memory_order_relaxed)){
        report_value_changed();
    }
    return true;
}
const IntegerEnumDatabase& IntegerEnumDropdownCell::database() const{
    return m_data->m_database;
}
void IntegerEnumDropdownCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    Data& data = *m_data;
    const EnumEntry* entry = data.m_database.find_slug(*str);
    if (entry != nullptr && entry->enabled){
        data.m_current.store(entry->enum_value, std::memory_order_relaxed);
        report_value_changed();
    }

    //  Backward compatibility with display names.
    entry = data.m_database.find_display(*str);
    if (entry != nullptr && entry->enabled){
        data.m_current.store(entry->enum_value, std::memory_order_relaxed);
        report_value_changed();
    }
}
JsonValue IntegerEnumDropdownCell::to_json() const{
    const Data& data = *m_data;
    return data.m_database.find(data.m_current.load(std::memory_order_relaxed))->slug;
}
void IntegerEnumDropdownCell::restore_defaults(){
    set_value(m_data->m_default);
}






}
