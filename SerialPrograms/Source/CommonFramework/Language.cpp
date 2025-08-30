/*  Language
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <map>
#include "Common/Cpp/Exceptions.h"
#include "Language.h"

namespace PokemonAutomation{



const std::map<Language, LanguageData> LANGUAGE_DATA{
    {Language::None,                {"none",    "None",                     0}},
    {Language::English,             {"eng",     "English",                  1. / 5}},
    {Language::Japanese,            {"jpn",     "Japanese",                 1. / 10}},
    {Language::Spanish,             {"spa",     "Spanish",                  1. / 5}},
    {Language::French,              {"fra",     "French",                   1. / 5}},
    {Language::German,              {"deu",     "German",                   1. / 5}},
    {Language::Italian,             {"ita",     "Italian",                  1. / 5}},
    {Language::Korean,              {"kor",     "Korean",                   1. / 10}},
    {Language::ChineseSimplified,   {"chi_sim", "Chinese (Simplified)",     1. / 100}},
    {Language::ChineseTraditional,  {"chi_tra", "Chinese (Traditional)",    1. / 100}},
};



bool LanguageSet::operator[](Language language) const{
    return m_set.find(language) != m_set.end();
}
void LanguageSet::operator+=(Language language){
    m_set.insert(language);
}
void LanguageSet::operator-=(Language language){
    m_set.erase(language);
}
void LanguageSet::operator+=(const LanguageSet& set){
    for (Language language : set.m_set){
        m_set.insert(language);
    }
}
void LanguageSet::operator-=(const LanguageSet& set){
    for (Language language : set.m_set){
        m_set.erase(language);
    }
}






const LanguageData& language_data(Language language){
    auto iter = LANGUAGE_DATA.find(language);
    if (iter == LANGUAGE_DATA.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Invalid Language Enum: " + std::to_string((int)language)
        );
    }
    return iter->second;
}

std::map<std::string, Language> build_code_to_enum_map(){
    std::map<std::string, Language> ret;
    for (auto& iter : LANGUAGE_DATA){
        ret.emplace(iter.second.code, iter.first);
    }
    return ret;
}
const std::map<std::string, Language> CODE_TO_ENUM = build_code_to_enum_map();


Language language_code_to_enum(const std::string& language){
    auto iter = CODE_TO_ENUM.find(language);
    if (iter == CODE_TO_ENUM.end()){
        throw InternalProgramError(
            nullptr, PA_CURRENT_FUNCTION,
            "Unknown Language Code: " + language
        );
    }
    return iter->second;
}

std::string language_warning(Language language){
    return "Please ensure that you have set the correct Game Language in the program settings.\n"
            "Current Language Set: " + language_data(language).name;

}


}
