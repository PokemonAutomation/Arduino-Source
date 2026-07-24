/*  Text Edit
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <set>
#include "Common/Cpp/ListenerSet.h"
#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "TextEditOption.h"

namespace PokemonAutomation{



struct TextEditOption::Data{
    const std::string m_label;
    const std::string m_default;
    const std::string m_placeholder_text;
    const bool m_report_all_text_changes;

    mutable SpinLock m_lock;
    std::string m_current;
    ListenerSet<FocusListener> focus_listeners;
    ListenerSet<AppendListener> append_listeners;

    Data(
        std::string label,
        std::string default_value,
        std::string placeholder_text,
        bool report_all_text_changes
    )
        : m_label(std::move(label))
        , m_default(std::move(default_value))
        , m_placeholder_text(std::move(placeholder_text))
        , m_report_all_text_changes(report_all_text_changes)
        , m_current(m_default)
    {}
};

void TextEditOption::add_focus_listener(FocusListener& listener){
    Data& data = *m_data;
    data.focus_listeners.add(listener);
}
void TextEditOption::remove_focus_listener(FocusListener& listener){
    Data& data = *m_data;
    data.focus_listeners.remove(listener);
}
void TextEditOption::report_focus_in(){
    Data& data = *m_data;
    data.focus_listeners.run_method(&FocusListener::focus_in);
}
void TextEditOption::add_append_listener(AppendListener& listener){
    Data& data = *m_data;
    data.append_listeners.add(listener);
}
void TextEditOption::remove_append_listener(AppendListener& listener){
    Data& data = *m_data;
    data.append_listeners.remove(listener);
}

TextEditOption::~TextEditOption() = default;
TextEditOption::TextEditOption(
    std::string label,
    LockMode lock_while_program_is_running,
    std::string default_value,
    std::string placeholder_text,
    bool signal_all_text_changes
)
    : ConfigOptionImpl<TextEditOption>(lock_while_program_is_running)
    , m_data(CONSTRUCT_TOKEN, std::move(label), std::move(default_value), std::move(placeholder_text), signal_all_text_changes)
{}
#if 0
std::unique_ptr<ConfigOption> TextEditOption::clone() const{
   std::unique_ptr<TextEditOption> ret(new TextEditOption(m_label, m_default, m_placeholder_text));
   ret->m_current = m_current;
   return ret;
}
#endif

const std::string& TextEditOption::label() const{
    return m_data->m_label;
}
const std::string& TextEditOption::placeholder_text() const{
    return m_data->m_placeholder_text;
}
bool TextEditOption::signal_all_text_changes() const{
    return m_data->m_report_all_text_changes;
}

TextEditOption::operator std::string() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
void TextEditOption::set(std::string x){
    {
        WriteSpinLock lg(m_data->m_lock);
        if (m_data->m_current == x){
            return;
        }
        m_data->m_current = std::move(x);
    }
    report_value_changed(this);
}
void TextEditOption::append(std::string x){
    if (x.empty()){
        return;
    }
    m_data->m_current += x;
    m_data->append_listeners.run_method(&AppendListener::on_append, std::move(x));
    report_value_changed(this);
}


void TextEditOption::load_json(const JsonValue& json){
    const std::string* str = json.to_string();
    if (str == nullptr){
        return;
    }
    set(*str);
}
JsonValue TextEditOption::to_json() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_current;
}
void TextEditOption::restore_defaults(){
    set(m_data->m_default);
}











}
