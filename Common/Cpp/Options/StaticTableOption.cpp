/*  Static Table Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
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
void StaticTableRow::report_program_state(bool program_is_running){
    for (const auto& item : m_options){
        item.second->report_program_state(program_is_running);
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




struct StaticTableOption::Data{
    const std::string m_label;
    const bool m_enable_saveload;
    std::vector<std::unique_ptr<StaticTableRow>> m_owners;
    std::vector<StaticTableRow*> m_table;

    bool m_finished = false;
    std::map<std::string, size_t> m_index_map;


    Data(std::string label, bool enable_saveload)
        : m_label(std::move(label))
        , m_enable_saveload(enable_saveload)
    {}

    void add_row(std::unique_ptr<StaticTableRow> row){
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
    void add_row(StaticTableRow* row){
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

    //  You must call this when done populating rows. Afterwards, you cannot add more.
    void finish_construction(){
        if (m_finished){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to finalize and already finalized table.");
        }
        m_finished = true;
    }

    void load_json(const JsonValue& json){
        if (!m_finished){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Attempted to use an unfinalized table.");
        }

        const JsonArray* array = json.to_array();
        if (array == nullptr){
            return;
        }

        for (const auto& item : *array){
            const JsonObject* obj = item.to_object();
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
    JsonValue to_json() const{
        JsonArray array;
        for (const StaticTableRow* row : m_table){
            JsonValue val = row->to_json();
            val.to_object_throw()["slug"] = row->slug();
            array.push_back(std::move(val));
        }
        return array;
    }
    std::string check_validity() const{
        for (const StaticTableRow* row : m_table){
            std::string error = row->check_validity();
            if (!error.empty()){
                return error;
            }
        }
        return std::string();
    }
    void restore_defaults(){
        for (StaticTableRow* row : m_table){
            row->restore_defaults();
        }
    }
};


StaticTableOption::~StaticTableOption() = default;
StaticTableOption::StaticTableOption(
    std::string label,
    LockMode lock_while_program_is_running,
    bool enable_saveload
)
    : ConfigOption(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, std::move(label), enable_saveload)
{}
void StaticTableOption::add_row(std::unique_ptr<StaticTableRow> row){
    m_data->add_row(std::move(row));
}
void StaticTableOption::add_row(StaticTableRow* row){
    m_data->add_row(row);
}
void StaticTableOption::finish_construction(){
    m_data->finish_construction();
}

const std::string& StaticTableOption::label() const{
    return m_data->m_label;
}
const std::vector<StaticTableRow*>& StaticTableOption::table() const{
    return m_data->m_table;
}

void StaticTableOption::load_json(const JsonValue& json){
    m_data->load_json(json);
}
JsonValue StaticTableOption::to_json() const{
    return m_data->to_json();
}
std::string StaticTableOption::check_validity() const{
    return m_data->check_validity();
}
void StaticTableOption::restore_defaults(){
    m_data->restore_defaults();
}
void StaticTableOption::report_program_state(bool program_is_running){
    for (StaticTableRow* row : m_data->m_table){
        row->report_program_state(program_is_running);
    }
}

bool StaticTableOption::saveload_enabled() const{
    return m_data->m_enable_saveload;
}


















}
