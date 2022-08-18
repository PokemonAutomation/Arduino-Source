/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Containers/Pimpl.tpp"
#include "Common/Cpp/Concurrency/SpinLock.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "StaticTextOption.h"

namespace PokemonAutomation{



struct StaticTextOption::Data{
    mutable SpinLock m_lock;
    std::string m_text;

    Data(std::string text)
        : m_text(std::move(text))
    {}
};


StaticTextOption::~StaticTextOption() = default;
StaticTextOption::StaticTextOption(std::string label)
    : m_data(CONSTRUCT_TOKEN, std::move(label))
{}
#if 0
std::unique_ptr<ConfigOption> StaticTextOption::clone() const{
    return std::unique_ptr<ConfigOption>(new StaticTextOption(m_text));
}
#endif
std::string StaticTextOption::text() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_text;
}
void StaticTextOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_text = std::move(label);
    }
    push_update();
}
void StaticTextOption::load_json(const JsonValue&){
}
JsonValue StaticTextOption::to_json() const{
    return JsonValue();
}






struct SectionDividerOption::Data{
    mutable SpinLock m_lock;
    std::string m_text;

    Data(std::string text)
        : m_text(std::move(text))
    {}
};

SectionDividerOption::~SectionDividerOption() = default;
SectionDividerOption::SectionDividerOption(std::string label)
    : m_data(CONSTRUCT_TOKEN, std::move(label))
{}
std::string SectionDividerOption::text() const{
    SpinLockGuard lg(m_data->m_lock);
    return m_data->m_text;
}
void SectionDividerOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_data->m_lock);
        m_data->m_text = std::move(label);
    }
    push_update();
}
void SectionDividerOption::load_json(const JsonValue&){
}
JsonValue SectionDividerOption::to_json() const{
    return JsonValue();
}










}
