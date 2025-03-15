/*  Editable Table Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <algorithm>
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "EditableTableOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


//EditableTableRow::EditableTableRow(void*)
//    : m_parent(nullptr)
//    , m_index((size_t)0 - 1)
//{}
EditableTableRow::EditableTableRow(EditableTableOption& parent_table)
    : m_parent_table(parent_table)
    , m_index((size_t)0 - 1)
{}
void EditableTableRow::add_option(ConfigOption& option, std::string serialization_string){
    m_options.emplace_back(std::move(serialization_string), &option);
}
void EditableTableRow::load_json(const JsonValue& json){
    if (m_options.size() == 1){
        m_options[0].second->load_json(json);
        return;
    }
    const JsonObject* obj = json.to_object();
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
JsonValue EditableTableRow::to_json() const{
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
std::string EditableTableRow::check_validity() const{
    for (const auto& item : m_options){
        std::string error = item.second->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
std::vector<ConfigOption*> EditableTableRow::make_cells(){
    std::vector<ConfigOption*> ret;
    ret.reserve(m_options.size());
    for (const auto& item : m_options){
        ret.emplace_back(item.second);
    }
    return ret;
}




EditableTableOption::EditableTableOption(
    std::string label,
    LockMode lock_while_running,
    std::vector<std::unique_ptr<EditableTableRow>> default_value
)
    : ConfigOption(lock_while_running)
    , m_label(std::move(label))
    , m_enable_saveload(true)
    , m_default(std::move(default_value))
{
    restore_defaults();
}
EditableTableOption::EditableTableOption(
    std::string label,
    LockMode lock_while_running,
    bool enable_saveload,
    std::vector<std::unique_ptr<EditableTableRow>> default_value
)
    : ConfigOption(lock_while_running)
    , m_label(std::move(label))
    , m_enable_saveload(enable_saveload)
    , m_default(std::move(default_value))
{
    restore_defaults();
}
void EditableTableOption::set_default(std::vector<std::unique_ptr<EditableTableRow>> default_value){
    WriteSpinLock lg(m_default_lock);
    m_default = std::move(default_value);
}
size_t EditableTableOption::current_rows() const{
    ReadSpinLock lg(m_current_lock);
    return m_current.size();
}
std::vector<std::shared_ptr<EditableTableRow>> EditableTableOption::current_refs() const{
    ReadSpinLock lg(m_current_lock);
    return m_current;
}

void EditableTableOption::clear(){
    WriteSpinLock lg(m_current_lock);
    m_current.clear();
}
void EditableTableOption::load_json(const JsonValue& json){
//    cout << "EditableTableOption::load_json(): " << this << endl;

    const JsonArray* array = json.to_array();
    if (array == nullptr){
        return;
    }
    {
        //  Pre-allocate seqnums.
        uint64_t seqnum = m_seqnum.fetch_add(array->size());

        //  Build table outside of lock first.
        //  If the table is built inside of the lock, you can get issues with deadlock. 
        //  The reason for this is that during the construction of the new table, 
        //  the listeners get triggered. Specifically, the row elements trigger
        //  their own Row.report_value_changed(), which may then propagate up with Table.report_value_changed(). 
        //  If the table's listener includes any functions that accesses the table (e.g. current_refs()),
        //  then this can result in deadlock since current_refs() can't access the table 
        //  until the table is finished building and releases the lock. 
        //  But the table can't finish building until current_refs() finishes.
        //  Building the table outside of the lock bypasses this issue since the listeners won't be 
        //  triggered in the first place.
        std::vector<std::shared_ptr<EditableTableRow>> table;
        for (const auto& item : *array){
            std::unique_ptr<EditableTableRow> row = make_row();
            row->m_seqnum = seqnum++;
            row->m_index = table.size();
            table.emplace_back(std::move(row));
            table.back()->load_json(item);
        }

        //  Now commit the table inside the lock.
        WriteSpinLock lg(m_current_lock);
        m_current = std::move(table);
    }
    report_value_changed(this);
}
JsonValue EditableTableOption::to_json() const{
    ReadSpinLock lg(m_current_lock);
    JsonArray array;
    for (const std::shared_ptr<EditableTableRow>& row : m_current){
        array.push_back(row->to_json());
    }
    return array;
}

std::string EditableTableOption::check_validity() const{
    ReadSpinLock lg(m_current_lock);
    for (const std::shared_ptr<EditableTableRow>& item : m_current){
        std::string error = item->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void EditableTableOption::restore_defaults(){
    {
        std::vector<std::shared_ptr<EditableTableRow>> tmp;
        {
            ReadSpinLock lg(m_default_lock);

            //  Pre-allocate seqnums.
            uint64_t seqnum = m_seqnum.fetch_add(m_default.size());

            //  Build table outside of lock first.
            //  If the table is built inside of the lock, you can get issues with deadlock.  
            //  (see load_json() for more details)           
            for (const std::unique_ptr<EditableTableRow>& item : m_default){
                tmp.emplace_back(item->clone());
                tmp.back()->m_seqnum = seqnum++;
                tmp.back()->m_index = tmp.size() - 1;
            }
        }

        //  Now commit the table inside the lock.
        WriteSpinLock lg(m_current_lock);
        m_current = std::move(tmp);
    }
    report_value_changed(this);
}



void EditableTableOption::insert_row(size_t index, std::unique_ptr<EditableTableRow> row){
    {
        WriteSpinLock lg(m_current_lock);
        index = std::min(index, m_current.size());
        row->m_seqnum = m_seqnum++;
        m_current.insert(m_current.begin() + index, std::move(row));
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    report_value_changed(this);
}
void EditableTableOption::append_row(std::unique_ptr<EditableTableRow> row){
    insert_row((size_t)-1, std::move(row));
}
void EditableTableOption::clone_row(const EditableTableRow& row){
    {
        size_t index = row.m_index;
        if (index == (size_t)0 - 1){
//            cout << "EditableTableOptionCore::clone_row(): Orphaned row" << endl;
            return;
        }

        //  Copy the row first.
        //  If the row is cloned inside of the lock, you can get issues with deadlock.
        //  (see load_json() for more details)            
        std::unique_ptr<EditableTableRow> new_row = row.clone();
        new_row->m_seqnum = m_seqnum++;

        //  Now add it to the table.
        WriteSpinLock lg(m_current_lock);
        index = std::min(index, m_current.size());
        m_current.insert(m_current.begin() + index, std::move(new_row));
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    report_value_changed(this);
}
void EditableTableOption::remove_row(EditableTableRow& row){
    {
        size_t index = row.m_index;
        if (index == (size_t)0 - 1){
//            cout << "EditableTableOptionCore::remove_row(): Orphaned row" << endl;
            return;
        }

        WriteSpinLock lg(m_current_lock);
        auto iter = m_current.begin() + index;
        (*iter)->m_index.store((size_t)0 - 1, std::memory_order_relaxed);
        m_current.erase(iter);
        size_t stop = m_current.size();
        for (size_t c = index; c < stop; c++){
            m_current[c]->m_index.store(c, std::memory_order_relaxed);
        }
    }
    report_value_changed(this);
}









}
