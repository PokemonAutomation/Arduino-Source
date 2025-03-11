/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "StaticTextOption.h"

namespace PokemonAutomation{



struct StaticTextOption::Data{
    mutable SpinLock m_lock;
    std::string m_text;
    bool m_text_wrapping;

    Data(std::string text, bool text_wrapping)
        : m_text(std::move(text))
        , m_text_wrapping(text_wrapping)
    {}
};


StaticTextOption::~StaticTextOption() = default;
StaticTextOption::StaticTextOption(std::string label, bool text_wrapping)
    : ConfigOption(LockMode::UNLOCK_WHILE_RUNNING)
    , m_data(CONSTRUCT_TOKEN, std::move(label), text_wrapping)
{}
#if 0
std::unique_ptr<ConfigOption> StaticTextOption::clone() const{
    return std::unique_ptr<ConfigOption>(new StaticTextOption(m_text));
}
#endif
bool StaticTextOption::text_wrapping() const{
    return m_data->m_text_wrapping;
}
std::string StaticTextOption::text() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_text;
}
void StaticTextOption::set_text(std::string label){
    {
        WriteSpinLock lg(m_data->m_lock);
        if (label == m_data->m_text){
            return;
        }
        m_data->m_text = std::move(label);
    }
    report_value_changed(this);
}
void StaticTextOption::load_json(const JsonValue&){
    report_value_changed(this);
}
JsonValue StaticTextOption::to_json() const{
    return JsonValue();
}






struct SectionDividerOption::Data{
    mutable SpinLock m_lock;
    std::string m_text;
    bool m_text_wrapping;

    Data(std::string text, bool text_wrapping)
        : m_text(std::move(text))
        , m_text_wrapping(text_wrapping)
    {}
};

SectionDividerOption::~SectionDividerOption() = default;
SectionDividerOption::SectionDividerOption(std::string label, bool text_wrapping)
    : m_data(CONSTRUCT_TOKEN, std::move(label), text_wrapping)
{}
bool SectionDividerOption::text_wrapping() const{
    return m_data->m_text_wrapping;
}
std::string SectionDividerOption::text() const{
    ReadSpinLock lg(m_data->m_lock);
    return m_data->m_text;
}
void SectionDividerOption::set_text(std::string label){
    {
        WriteSpinLock lg(m_data->m_lock);
        if (label == m_data->m_text){
            return;
        }
        m_data->m_text = std::move(label);
    }
    report_value_changed(this);
}
void SectionDividerOption::load_json(const JsonValue&){
    report_value_changed(this);
}
JsonValue SectionDividerOption::to_json() const{
    return JsonValue();
}










}
