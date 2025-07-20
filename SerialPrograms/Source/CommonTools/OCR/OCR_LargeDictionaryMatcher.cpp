/*  Large Database Matcher
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include "Common/Cpp/Exceptions.h"
#include "CommonFramework/Globals.h"
#include "OCR_StringNormalization.h"
#include "OCR_TextMatcher.h"
#include "OCR_LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{



LargeDictionaryMatcher::LargeDictionaryMatcher(
    const std::string& json_file_prefix,
    const std::set<std::string>* subset,
    bool first_only
){
    std::string prefix = RESOURCE_PATH() + json_file_prefix;
    for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
        Language language = (Language)c;
        const LanguageData& data = language_data(language);
        const std::string& code = data.code;
        try{
            m_database.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(language),
                std::forward_as_tuple(prefix + code + ".json", subset, data.random_match_chance, first_only)
            );
            m_languages += language;
        }catch (FileException&){}
    }
}

void LargeDictionaryMatcher::save(Language language, const std::string& json_path) const{
    dictionary(language).save_json(json_path);
}






}
}
