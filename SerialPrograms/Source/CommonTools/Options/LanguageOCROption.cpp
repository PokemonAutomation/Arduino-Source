/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/Logging/Logger.h"
#include "CommonTools/OCR/OCR_RawOCR.h"
#include "LanguageOCROption.h"

//#include <iostream>
//using std::cout;
//using std::endl;

namespace PokemonAutomation{
namespace OCR{



LanguageOCRCell::LanguageOCRCell(
    const LanguageSet& languages,
    LockMode lock_while_running,
    bool required
)
    : ConfigOption(lock_while_running)
    , m_default(0)
    , m_current(0)
{
    size_t index = 0;
    if (!required && !languages[Language::None]){
        m_case_list.emplace_back(
            Language::None,
            true
        );
        m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(Language::None),
            std::forward_as_tuple(index)
        );
        index++;
    }
    for (Language language : languages){
        m_case_list.emplace_back(
            language,
            language == Language::None || OCR::language_available(language)
        );
        m_case_map.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(language),
            std::forward_as_tuple(index)
        );
        index++;
    }
}


void LanguageOCRCell::set(Language language){
    auto iter = m_case_map.find(language);
    if (iter == m_case_map.end()){
        return;
    }
    if (iter->second != m_current.exchange(iter->second, std::memory_order_relaxed)){
        report_value_changed(this);
    }
}


void LanguageOCRCell::load_json(const JsonValue& json){
    const std::string* str = json.to_string();
    if (str == nullptr){
        return;
    }
    Language language;
    try{
        language = language_code_to_enum(*str);
    }catch (const InternalProgramError& e){
        global_logger_tagged().log("Error loading json for language OCR option: " + e.message(), COLOR_RED);
        return;
    }

    auto iter = m_case_map.find(language);
    if (iter == m_case_map.end()){
        const auto& lan_data = language_data(language);
        global_logger_tagged().log(
            "Warning: no language data for loaded language option " + lan_data.name + " (" + lan_data.code + ").",
            COLOR_RED
        );
        return;
    }
    m_current.store(iter->second, std::memory_order_relaxed);
    report_value_changed(this);
}
JsonValue LanguageOCRCell::to_json() const{
    return language_data((Language)*this).code;
}

std::string LanguageOCRCell::check_validity() const{
    return m_case_list[m_current.load(std::memory_order_relaxed)].second ? std::string() : "Language data is not available.";
}
void LanguageOCRCell::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    report_value_changed(this);
}











LanguageOCROption::LanguageOCROption(
    std::string label,
    const LanguageSet& languages,
    LockMode lock_while_running,
    bool required
)
    : LanguageOCRCell(languages, lock_while_running, required)
    , m_label(std::move(label))
{}





}
}

