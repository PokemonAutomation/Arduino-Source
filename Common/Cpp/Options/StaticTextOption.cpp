/*  Static Text
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Json/JsonValue.h"
#include "Common/Qt/Options/ConfigWidget.h"
#include "StaticTextOption.h"

namespace PokemonAutomation{




StaticTextOption::StaticTextOption(std::string label)
    : m_text(std::move(label))
{}
#if 0
std::unique_ptr<ConfigOption> StaticTextOption::clone() const{
    return std::unique_ptr<ConfigOption>(new StaticTextOption(m_text));
}
#endif
std::string StaticTextOption::text() const{
    SpinLockGuard lg(m_lock);
    return m_text;
}
void StaticTextOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_lock);
        m_text = std::move(label);
    }
    push_update();
}
void StaticTextOption::load_json(const JsonValue&){
}
JsonValue StaticTextOption::to_json() const{
    return JsonValue();
}



SectionDividerOption::SectionDividerOption(std::string label)
    : m_text(std::move(label))
{}
#if 0
std::unique_ptr<ConfigOption> SectionDividerOption::clone() const{
    return std::unique_ptr<ConfigOption>(new SectionDividerOption(m_text));
}
#endif
std::string SectionDividerOption::text() const{
    SpinLockGuard lg(m_lock);
    return m_text;
}
void SectionDividerOption::set_text(std::string label){
    {
        SpinLockGuard lg(m_lock);
        m_text = std::move(label);
    }
    push_update();
}
void SectionDividerOption::load_json(const JsonValue&){
}
JsonValue SectionDividerOption::to_json() const{
    return JsonValue();
}










}
