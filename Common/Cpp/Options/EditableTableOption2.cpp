/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <algorithm>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "EditableTableOption2.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{


EditableTableRow2::EditableTableRow2()
    : m_index((size_t)0 - 1)
{}
void EditableTableRow2::add_option(ConfigOption& option, std::string serialization_string){
    m_options.emplace_back(std::move(serialization_string), &option);
}
void EditableTableRow2::load_json(const JsonValue& json){
    if (m_options.size() == 1){
        m_options[0].second->load_json(json);
        return;
    }
    const JsonObject* obj = json.get_object();
    if (obj == nullptr){
        return;
    }
    for (auto& item : m_options){
        if (!item.first.empty()){
            const JsonValue* value = obj->get_value(item.first);
            if (value){
                item.second->load_json(*value);
            }
        }
    }
}
JsonValue EditableTableRow2::to_json() const{
    if (m_options.size() == 1){
        return m_options[0].second->to_json();
    }
    JsonObject obj;
    for (auto& item : m_options){
        if (!item.first.empty()){
            obj[item.first] = item.second->to_json();
        }
    }
    return obj;
}
std::string EditableTableRow2::check_validity() const{
    for (const auto& item : m_options){
        std::string error = item.second->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
std::vector<ConfigOption*> EditableTableRow2::make_cells(){
    std::vector<ConfigOption*> ret;
    ret.reserve(m_options.size());
    for (const auto& item : m_options){
        ret.emplace_back(item.second);
    }
    return ret;
}




EditableTableOption2::EditableTableOption2(
    std::string label,
    std::vector<std::unique_ptr<EditableTableRow2>> default_value
)
    : m_label(std::move(label))
    , m_default(std::move(default_value))
{
    restore_defaults();
}
size_t EditableTableOption2::current_rows() const{
    SpinLockGuard lg(m_lock);
    return m_current.size();
}
std::vector<std::shared_ptr<EditableTableRow2>> EditableTableOption2::current_refs() const{
    SpinLockGuard lg(m_lock);
    return m_current;
}

void EditableTableOption2::load_json(const JsonValue& json){
    const JsonArray* array = json.get_array();
    if (array == nullptr){
        return;
    }
    {
        SpinLockGuard lg(m_lock);
        std::vector<std::shared_ptr<EditableTableRow2>> table;
        for (const auto& item : *array){
            std::unique_ptr<EditableTableRow2> row = make_row();
            row->m_seqnum = m_seqnum++;
            row->m_index = table.size();
            table.emplace_back(std::move(row));
            table.back()->load_json(item);
        }
        m_current = std::move(table);
    }
    push_update();
}
JsonValue EditableTableOption2::to_json() const{
    SpinLockGuard lg(m_lock);
    JsonArray array;
    for (const std::shared_ptr<EditableTableRow2>& row : m_current){
        array.push_back(row->to_json());
    }
    return array;
}

std::string EditableTableOption2::check_validity() const{
    SpinLockGuard lg(m_lock);
    for (const std::shared_ptr<EditableTableRow2>& item : m_current){
        std::string error = item->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void EditableTableOption2::restore_defaults(){
    {
        std::vector<std::shared_ptr<EditableTableRow2>> tmp;
        SpinLockGuard lg(m_lock);
        for (const std::unique_ptr<EditableTableRow2>& item : m_default){
            tmp.emplace_back(item->clone());
            tmp.back()->m_seqnum = m_seqnum++;
            tmp.back()->m_index = tmp.size() - 1;
        }
        m_current = std::move(tmp);
    }
    push_update();
}



void EditableTableOption2::insert_row(size_t index, std::unique_ptr<EditableTableRow2> row){
    {
        SpinLockGuard lg(m_lock);
        index = std::min(index, m_current.size());
        row->m_seqnum = m_seqnum++;
        m_current.insert(m_current.begin() + index, std::move(row));
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    push_update();
}
void EditableTableOption2::clone_row(const EditableTableRow2& row){
    {
        SpinLockGuard lg(m_lock);
        size_t index = row.m_index;
        if (index == (size_t)0 - 1){
            cout << "EditableTableOptionCore::clone_row(): Orphaned row" << endl;
            return;
        }
        std::unique_ptr<EditableTableRow2> new_row = row.clone();
        new_row->m_seqnum = m_seqnum++;
        m_current.insert(m_current.begin() + index, std::move(new_row));
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    push_update();
}
void EditableTableOption2::remove_row(EditableTableRow2& row){
    {
        SpinLockGuard lg(m_lock);
        size_t index = row.m_index;
        if (index == (size_t)0 - 1){
            cout << "EditableTableOptionCore::remove_row(): Orphaned row" << endl;
            return;
        }
        auto iter = m_current.begin() + index;
        (*iter)->m_index.store((size_t)0 - 1, std::memory_order_relaxed);
        m_current.erase(iter);
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    push_update();
}









}
