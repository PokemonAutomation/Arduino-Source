/*  Batch Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <string>
#include <vector>
#include "Common/Compiler.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Containers/FixedLimitVector.tpp"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Cpp/Json/JsonObject.h"
#include "BatchOption.h"

namespace PokemonAutomation{


struct BatchOption::Data{
    const bool m_horizontal;
    std::vector<std::pair<ConfigOption*, std::string>> m_options;

    Data(bool horizontal)
        : m_horizontal(horizontal)
    {}
};



BatchOption::~BatchOption() = default;
BatchOption::BatchOption(LockMode lock_while_program_is_running, bool horizontal)
    : ConfigOption(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, horizontal)
{}
void BatchOption::add_option(ConfigOption& option, std::string serialization_string){
    m_data->m_options.emplace_back(&option, std::move(serialization_string));
}

void BatchOption::load_json(const JsonValue& json){
    const JsonObject* obj = json.to_object();
    if (obj == nullptr){
        return;
    }
    for (auto& item : m_data->m_options){
        if (!item.second.empty()){
            const JsonValue* value = obj->get_value(item.second);
            if (value){
                item.first->load_json(*value);
            }
        }
    }
}
JsonValue BatchOption::to_json() const{
    JsonObject obj;
    for (auto& item : m_data->m_options){
        if (!item.second.empty()){
            obj[item.second] = item.first->to_json();
        }
    }
    return obj;
}

std::string BatchOption::check_validity() const{
    for (const auto& item : m_data->m_options){
        std::string error = item.first->check_validity();
        if (!error.empty()){
            return error;
        }
    }
    return std::string();
}
void BatchOption::restore_defaults(){
    for (const auto& item : m_data->m_options){
        item.first->restore_defaults();
    }
}
void BatchOption::reset_state(){
    for (const auto& item : m_data->m_options){
        item.first->reset_state();
    }
}
void BatchOption::report_program_state(bool program_is_running){
    ConfigOption::report_program_state(program_is_running);
    for (const auto& item : m_data->m_options){
        item.first->report_program_state(program_is_running);
    }
}

bool BatchOption::horizontal() const{
    return m_data->m_horizontal;
}
FixedLimitVector<ConfigOption*> BatchOption::options() const{
    FixedLimitVector<ConfigOption*> ret(m_data->m_options.size());
    for (const auto& item : m_data->m_options){
        ret.emplace_back(item.first);
    }
    return ret;
}




template class FixedLimitVector<ConfigOption*>;




}
