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
IntegerEnumDropdownCell::IntegerEnumDropdownCell(const IntegerEnumDatabase& database, size_t default_value, size_t current_value)
    : m_data(CONSTRUCT_TOKEN, database, default_value, current_value)
{}
IntegerEnumDropdownCell::IntegerEnumDropdownCell(const IntegerEnumDatabase& database, size_t default_value)
    : m_data(CONSTRUCT_TOKEN, database, default_value, default_value)
{}
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
    data.m_current.store(value, std::memory_order_relaxed);
    push_update();
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
        push_update();
    }

    //  Backward compatibility with display names.
    entry = data.m_database.find_display(*str);
    if (entry != nullptr && entry->enabled){
        data.m_current.store(entry->enum_value, std::memory_order_relaxed);
        push_update();
    }
}
JsonValue IntegerEnumDropdownCell::to_json() const{
    const Data& data = *m_data;
    return data.m_database.find(data.m_current.load(std::memory_order_relaxed))->slug;
}
void IntegerEnumDropdownCell::restore_defaults(){
    Data& data = *m_data;
    data.m_current.store(data.m_default, std::memory_order_relaxed);
    push_update();
}






}
