/*  Static Table Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "StaticTableOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


StaticTableRow::StaticTableRow(std::string slug)
    : m_slug(std::move(slug))
{}
void StaticTableRow::add_option(ConfigOption& option, std::string serialization_string){
    m_options.emplace_back(std::move(serialization_string), &option);
}
void StaticTableRow::load_json(const JsonValue& json){
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
JsonValue StaticTableRow::to_json() const{
    JsonObject obj;
    for (auto& item : m_options){
        if (!item.first.empty()){
            obj[item.first] = item.second->to_json();
        }
    }
    return obj;
}
std::string StaticTableRow::check_validity() const{
    for (const auto& item : m_options){
        std::string error = item.second->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void StaticTableRow::restore_defaults(){
    for (const auto& item : m_options){
        item.second->restore_defaults();
    }
}
std::vector<ConfigOption*> StaticTableRow::make_cells(){
    std::vector<ConfigOption*> ret;
    ret.reserve(m_options.size());
    for (const auto& item : m_options){
        ret.emplace_back(item.second);
    }
    return ret;
}





StaticTableOption::StaticTableOption(std::string label)
    : m_label(std::move(label))
{}
void StaticTableOption::add_row(std::unique_ptr<StaticTableRow> row){
    if (m_finished){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add row to finalized table.");
    }

    const std::string& slug = row->slug();

    bool owners_pushed = false;
    bool table_pushed = false;
    try{
        m_owners.emplace_back(std::move(row));
        owners_pushed = true;
        m_table.emplace_back(m_owners.back().get());
        table_pushed = true;

        auto ret = m_index_map.emplace(slug, m_index_map.size());
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Slug: " + slug);
        }
    }catch (...){
        if (owners_pushed){
            m_owners.pop_back();
        }
        if (table_pushed){
            m_table.pop_back();
        }
        throw;
    }
}
void StaticTableOption::add_row(StaticTableRow* row){
    if (m_finished){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to add row to finalized table.");
    }

    const std::string& slug = row->slug();

    bool table_pushed = false;
    try{
        m_table.emplace_back(row);
        table_pushed = true;

        auto ret = m_index_map.emplace(slug, m_index_map.size());
        if (!ret.second){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Duplicate Slug: " + slug);
        }
    }catch (...){
        if (table_pushed){
            m_table.pop_back();
        }
        throw;
    }
}
void StaticTableOption::finish_construction(){
    if (m_finished){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to finalize and already finalized table.");
    }
    m_finished = true;
}

void StaticTableOption::load_json(const JsonValue& json){
    if (!m_finished){
        throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to use an unfinalized table.");
    }

    const JsonArray* array = json.get_array();
    if (array == nullptr){
        return;
    }

    for (const auto& item : *array){
        const JsonObject* obj = item.get_object();
        if (obj == nullptr){
            continue;
        }
        const std::string* slug = obj->get_string("slug");
        if (slug == nullptr){
            slug = obj->get_string("Slug");
        }
        if (slug == nullptr){
            continue;
        }
        auto iter = m_index_map.find(*slug);
        if (iter == m_index_map.end()){
//            cout << "slug not found: " << *slug << endl;
            continue;
        }
        m_table[iter->second]->load_json(item);
    }
}
JsonValue StaticTableOption::to_json() const{
    JsonArray array;
    for (const StaticTableRow* row : m_table){
        JsonValue val = row->to_json();
        val.get_object_throw()["slug"] = row->slug();
        array.push_back(std::move(val));
    }
    return array;
}
std::string StaticTableOption::check_validity() const{
    for (const StaticTableRow* row : m_table){
        std::string error = row->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void StaticTableOption::restore_defaults(){
    for (StaticTableRow* row : m_table){
        row->restore_defaults();
    }
}



















}
