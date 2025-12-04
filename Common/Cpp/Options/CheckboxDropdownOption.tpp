/*  Checkbox Dropdown Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 *      Same as EnumDropdown, but allows multiple selections.
 *
 */

#ifndef PokemonAutomation_Options_CheckboxDropdownOption_TPP
#define PokemonAutomation_Options_CheckboxDropdownOption_TPP

#include "Common/Cpp/Json/JsonArray.h"
#include "CheckboxDropdownDatabase.h"
#include "CheckboxDropdownOption.h"

namespace PokemonAutomation{



template <typename FlagEnum>
CheckboxDropdownCell<FlagEnum>::CheckboxDropdownCell(
    std::string label,
    const Database& database,
    LockMode lock_while_running,
    FlagEnum default_value
)
    : CheckboxDropdownBase(std::move(label))
    , m_database(database)
    , m_default(default_value)
    , m_current(default_value)
{}
template <typename FlagEnum>
CheckboxDropdownCell<FlagEnum>::CheckboxDropdownCell(
    std::string label,
    const Database& database,
    LockMode lock_while_running,
    FlagEnum default_value, FlagEnum current_value
)
    : CheckboxDropdownBase(std::move(label))
    , m_database(database)
    , m_default(default_value)
    , m_current(current_value)
{}


template <typename FlagEnum>
FlagEnum CheckboxDropdownCell<FlagEnum>::current_value() const{
    ReadSpinLock lg(m_lock);
    return m_current;
}


template <typename FlagEnum>
std::string CheckboxDropdownCell<FlagEnum>::current_label() const{
    ReadSpinLock lg(m_lock);
    if (!m_label.empty()){
        return m_label;
    }

    std::string ret;
    bool first = true;
    for (const FlagEnumEntry<FlagEnum>& entry : m_database){
        if (is_empty(m_current & entry.value)){
            continue;
        }
        if (!first){
            ret += ", ";
        }
        first = false;

        ret += entry.display;
    }

    if (ret.size() > 30){
        ret = "( ... )";
    }

    return ret;
}
template <typename FlagEnum>
bool CheckboxDropdownCell<FlagEnum>::is_set(FlagEnum value) const{
    ReadSpinLock lg(m_lock);
    return !is_empty(value & m_current);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::replace_all(FlagEnum value){
    {
        WriteSpinLock lg(m_lock);
        if (m_current == value){
            return;
        }
        m_current = value;
    }
    report_value_changed(this);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::clear(){
    replace_all(empty_value((FlagEnum*)nullptr));
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::set_flag(FlagEnum value){
    {
        WriteSpinLock lg(m_lock);
        if (!is_empty(m_current & value)){
            return;
        }
        m_current |= value;
    }
    report_value_changed(this);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::clear_flag(FlagEnum value){
    {
        WriteSpinLock lg(m_lock);
        if (is_empty(m_current & value)){
            return;
        }
        m_current |= value;
        m_current ^= value;
    }
    report_value_changed(this);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::toggle_flag(FlagEnum value){
    {
        WriteSpinLock lg(m_lock);
        m_current ^= value;
    }
    report_value_changed(this);
}

template <typename FlagEnum>
size_t CheckboxDropdownCell<FlagEnum>::size() const{
    return m_database.size();
}
template <typename FlagEnum>
const std::string& CheckboxDropdownCell<FlagEnum>::name_at_index(size_t index) const{
    return m_database[index].display;
}
template <typename FlagEnum>
bool CheckboxDropdownCell<FlagEnum>::operator[](size_t index) const{
    return is_set(m_database[index].value);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::set_index(size_t index){
    set_flag(m_database[index].value);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::clear_index(size_t index){
    clear_flag(m_database[index].value);
}
template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::toggle_index(size_t index){
    toggle_flag(m_database[index].value);
}




template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::load_json(const JsonValue& json){
    m_current = empty_value((FlagEnum*)nullptr);
    const JsonArray& array = json.to_array_throw();
    for (const JsonValue& item : array){
        const std::string slug = item.to_string_throw();
        const FlagEnum* flag = m_database.find_slug(slug);
        if (flag){
            m_current |= *flag;
        }
    }
}
template <typename FlagEnum>
JsonValue CheckboxDropdownCell<FlagEnum>::to_json() const{
    JsonArray ret;
    for (const FlagEnumEntry<FlagEnum>& value : m_database){
        if (is_set(value.value)){
            ret.push_back(value.slug);
        }
    }
    return ret;
}

template <typename FlagEnum>
void CheckboxDropdownCell<FlagEnum>::restore_defaults(){
    WriteSpinLock lg(m_lock);
    m_current = m_default;
}






}
#endif
