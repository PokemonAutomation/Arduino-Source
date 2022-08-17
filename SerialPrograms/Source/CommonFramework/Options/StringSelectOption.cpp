/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCompleter>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/NoWheelComboBox.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "StringSelectOption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{




size_t StringSelectDatabase::search_index_by_slug(const std::string& slug) const{
    auto iter = m_slug_to_entry.find(slug);
    if (iter == m_slug_to_entry.end()){
        return (size_t)0 - 1;
    }
    return iter->second;
}
size_t StringSelectDatabase::search_index_by_name(const std::string& display_name) const{
    auto iter = m_display_name_to_entry.find(display_name);
    if (iter == m_display_name_to_entry.end()){
        return (size_t)0 - 1;
    }
    return iter->second;
}
void StringSelectDatabase::add_entry(StringSelectEntry entry){
    size_t index = m_list.size();
    StringSelectEntry& item = m_list.emplace_back(std::move(entry));

    std::map<std::string, size_t>::const_iterator iter0 = m_slug_to_entry.end();
    std::map<std::string, size_t>::const_iterator iter1 = m_display_name_to_entry.end();

    try{
        auto ret0 = m_slug_to_entry.emplace(item.slug, index);
        if (ret0.second){
            iter0 = ret0.first;
        }else{
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Slug: " + item.slug);
        }
        auto ret1 = m_display_name_to_entry.emplace(item.display_name, index);
        if (ret1.second){
            iter1 = ret1.first;
        }else{
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Display Name: " + item.display_name);
        }
    }catch (...){
        m_list.pop_back();
        if (iter0 != m_slug_to_entry.end()){
            m_slug_to_entry.erase(iter0);
        }
        if (iter1 != m_display_name_to_entry.end()){
            m_slug_to_entry.erase(iter1);
        }
        throw;
    }
}


StringSelectCell::StringSelectCell(const StringSelectDatabase& database, size_t default_index)
    : m_database(database)
    , m_default(default_index)
    , m_index(default_index)
{
    if (default_index >= database.case_list().size()){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Index: " + std::to_string(default_index));
    }
}
StringSelectCell::StringSelectCell(const StringSelectDatabase& database, const std::string& default_slug)
    : m_database(database)
    , m_default(default_slug == "" ? 0 : database.search_index_by_slug(default_slug))
    , m_index(m_default)
{
    if (m_default == (size_t)0 - 1){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Slug: " + default_slug);
    }
}
std::string StringSelectCell::set_by_slug(const std::string& slug){
    size_t index = m_database.search_index_by_slug(slug);
    if (index == (size_t)0 - 1){
        return "Invalid Slug: " + slug;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
    return "";
}
std::string StringSelectCell::set_by_name(const std::string& display_name){
    size_t index = m_database.search_index_by_name(display_name);
    if (index == (size_t)0 - 1){
        return "Invalid Name: " + display_name;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
    return "";
}
void StringSelectCell::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    size_t index = m_database.search_index_by_slug(*str);
    if (index == (size_t)0 - 1){
        return;
    }
    m_index.store(index, std::memory_order_relaxed);
    push_update();
}
JsonValue StringSelectCell::to_json() const{
    return m_database.case_list()[m_index.load(std::memory_order_relaxed)].slug;
}
void StringSelectCell::restore_defaults(){
    m_index.store(m_default, std::memory_order_relaxed);
    push_update();
}





















}
