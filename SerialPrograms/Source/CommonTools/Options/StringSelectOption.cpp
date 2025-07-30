/*  String Select
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QCompleter>
#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonArray.h"
#include "StringSelectOption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{


struct StringSelectDatabase::Data{
    size_t m_longest_text_length = 0;
    std::vector<StringSelectEntry> m_list;
    std::map<std::string, size_t> m_slug_to_entry;
    std::map<std::string, size_t> m_display_name_to_entry;

    size_t search_index_by_slug(const std::string& slug) const{
        auto iter = m_slug_to_entry.find(slug);
        if (iter == m_slug_to_entry.end()){
            return (size_t)0 - 1;
        }
        return iter->second;
    }
    size_t search_index_by_name(const std::string& display_name) const{
        auto iter = m_display_name_to_entry.find(display_name);
        if (iter == m_display_name_to_entry.end()){
            return (size_t)0 - 1;
        }
        return iter->second;
    }
    void add_entry(StringSelectEntry entry){
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

        m_longest_text_length = std::max(m_longest_text_length, item.display_name.size());
    }
    size_t size() const { return m_list.size(); }
};



StringSelectDatabase::~StringSelectDatabase() = default;
StringSelectDatabase::StringSelectDatabase(StringSelectDatabase&&) = default;
StringSelectDatabase& StringSelectDatabase::operator=(StringSelectDatabase&&) = default;
StringSelectDatabase::StringSelectDatabase(const StringSelectDatabase&) = default;
StringSelectDatabase& StringSelectDatabase::operator=(const StringSelectDatabase&) = default;
StringSelectDatabase::StringSelectDatabase()
    : m_data(CONSTRUCT_TOKEN)
{}

size_t StringSelectDatabase::longest_text_length() const{
    return m_data->m_longest_text_length;
}
const std::vector<StringSelectEntry>& StringSelectDatabase::case_list() const{
    return m_data->m_list;
}
const StringSelectEntry& StringSelectDatabase::operator[](size_t index) const{
    return m_data->m_list[index];
}
size_t StringSelectDatabase::search_index_by_slug(const std::string& slug) const{
    return m_data->search_index_by_slug(slug);
}
size_t StringSelectDatabase::search_index_by_name(const std::string& display_name) const{
    return m_data->search_index_by_name(display_name);
}
void StringSelectDatabase::add_entry(StringSelectEntry entry){
    m_data->add_entry(std::move(entry));
}
size_t StringSelectDatabase::size() const{
    return m_data->size();
}

StringSelectDatabase create_string_select_database(const std::vector<std::string>& slugs){
    StringSelectDatabase database;
    for (const std::string& slug : slugs){
        // slug name is also the display name
        database.add_entry(StringSelectEntry(slug, slug));
    }
    return database;
}

bool load_json_to_string_select_database(const JsonValue& json, StringSelectDatabase& database){
    const JsonArray* json_array = json.to_array();
    std::vector<std::string> slugs;
    if (json_array == nullptr){
        return false;
    }
    for (const JsonValue& element : *json_array){
        const std::string* str = element.to_string();
        if (str == nullptr){
            return false;
        }
        slugs.push_back(*str);
    }
    database = create_string_select_database(slugs);
    return true;
}




struct StringSelectCell::Data{
    const StringSelectDatabase& m_database;
    const size_t m_default;
    std::atomic<size_t> m_index;

    Data(const StringSelectDatabase& database, size_t default_index)
        : m_database(database)
        , m_default(default_index)
        , m_index(default_index)
    {
        if (default_index >= database.case_list().size()){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Index: " + std::to_string(default_index));
        }
    }
    Data(const StringSelectDatabase& database, const std::string& default_slug)
        : m_database(database)
        , m_default(default_slug == "" ? 0 : database.search_index_by_slug(default_slug))
        , m_index(m_default)
    {
        if (m_default == (size_t)0 - 1){
            throw InternalProgramError(nullptr, PA_CURRENT_FUNCTION, "Invalid Default Slug: " + default_slug);
        }
    }

    bool load_json(const JsonValue& json){
        const std::string* str = json.to_string();
        if (str == nullptr){
            return false;
        }
        size_t index = m_database.search_index_by_slug(*str);
        if (index == (size_t)0 - 1){
            return false;
        }
        m_index.store(index, std::memory_order_relaxed);
        return true;
    }
    JsonValue to_json() const{
        return m_database.case_list()[m_index.load(std::memory_order_relaxed)].slug;
    }
    void restore_defaults(){
        m_index.store(m_default, std::memory_order_relaxed);
    }
};





StringSelectCell::~StringSelectCell() = default;


StringSelectCell::StringSelectCell(
    const StringSelectDatabase& database,
    LockMode lock_while_running,
    size_t default_index
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, database, default_index)
{}
StringSelectCell::StringSelectCell(
    const StringSelectDatabase& database,
    LockMode lock_while_running,
    const std::string& default_slug
)
    : ConfigOption(lock_while_running)
    , m_data(CONSTRUCT_TOKEN, database, default_slug)
{}


size_t StringSelectCell::default_index() const{
    return m_data->m_default;
}
const std::string& StringSelectCell::default_slug() const{
    return m_data->m_database.case_list()[m_data->m_default].slug;
}

size_t StringSelectCell::index() const{
    return m_data->m_index.load(std::memory_order_relaxed);
}

void StringSelectCell::set_by_index(size_t index){
    if (index >= m_data->m_database.case_list().size()){
        index = m_data->m_default;
    }
    if (index != m_data->m_index.exchange(index, std::memory_order_relaxed)){
        report_value_changed(this);
    }
}
std::string StringSelectCell::set_by_slug(const std::string& slug){
    size_t index = m_data->m_database.search_index_by_slug(slug);
    if (index == (size_t)0 - 1){
        return "Invalid Slug: " + slug;
    }
    if (index != m_data->m_index.exchange(index, std::memory_order_relaxed)){
        report_value_changed(this);
    }
    return "";
}
std::string StringSelectCell::set_by_name(const std::string& display_name){
    size_t index = m_data->m_database.search_index_by_name(display_name);
    if (index == (size_t)0 - 1){
        return "Invalid Name: " + display_name;
    }
    if (index != m_data->m_index.exchange(index, std::memory_order_relaxed)){
        report_value_changed(this);
    }
    return "";
}

const StringSelectDatabase& StringSelectCell::database() const{
    return m_data->m_database;
}

void StringSelectCell::load_json(const JsonValue& json){
    if (m_data->load_json(json)){
        report_value_changed(this);
    }
}
JsonValue StringSelectCell::to_json() const{
    return m_data->to_json();
}
void StringSelectCell::restore_defaults(){
    m_data->restore_defaults();
    report_value_changed(this);
}





















}
