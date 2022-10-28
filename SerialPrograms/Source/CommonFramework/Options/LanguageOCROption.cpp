/*  Language OCR Option
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "Common/Cpp/Json/JsonValue.h"
#include "CommonFramework/OCR/OCR_RawOCR.h"
#include "LanguageOCROption.h"

#include <iostream>
using std::cout;
using std::endl;

namespace PokemonAutomation{
namespace OCR{



LanguageOCR::LanguageOCR(std::string label, const LanguageSet& languages, bool required)
    : m_label(std::move(label))
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


void LanguageOCR::set(Language language){
    auto iter = m_case_map.find(language);
    if (iter == m_case_map.end()){
        return;
    }
    m_current.store(iter->second, std::memory_order_relaxed);
    report_value_changed();
}


void LanguageOCR::load_json(const JsonValue& json){
    const std::string* str = json.get_string();
    if (str == nullptr){
        return;
    }
    Language language;
    try{
        language = language_code_to_enum(*str);
    }catch (const InternalProgramError&){
        return;
    }

    auto iter = m_case_map.find(language);
    if (iter == m_case_map.end()){
        return;
    }
    m_current.store(iter->second, std::memory_order_relaxed);
    report_value_changed();
}
JsonValue LanguageOCR::to_json() const{
    return language_data((Language)*this).code;
}

std::string LanguageOCR::check_validity() const{
    return m_case_list[m_current.load(std::memory_order_relaxed)].second ? std::string() : "Language data is not available.";
}
void LanguageOCR::restore_defaults(){
    m_current.store(m_default, std::memory_order_relaxed);
    report_value_changed();
}





}
}

