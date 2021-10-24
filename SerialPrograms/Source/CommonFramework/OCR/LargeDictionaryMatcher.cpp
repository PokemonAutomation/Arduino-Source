/*  Large Database Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "StringNormalization.h"
#include "TextMatcher.h"
#include "LargeDictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{



LargeDictionaryMatcher::LargeDictionaryMatcher(
    const QString& json_file_prefix,
    const std::set<std::string>* subset,
    bool first_only
){
    QString prefix = RESOURCE_PATH() + json_file_prefix;
    for (size_t c = 1; c < (size_t)Language::EndOfList; c++){
        Language language = (Language)c;
        const LanguageData& data = language_data(language);
        const std::string& code = data.code;
        try{
            m_database.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(language),
                std::forward_as_tuple(prefix + QString::fromStdString(code) + ".json", subset, data.random_match_chance, first_only)
            );
            m_languages += language;
        }catch (FileException&){}
    }
}

void LargeDictionaryMatcher::save(Language language, const QString& json_path) const{
    dictionary(language).save_json(json_path);
}

#if 0
void LargeDictionaryMatcher::update(Language language) const{
    const std::string& code = language_data(language).code;
    save(language, m_prefix + "-" + QString::fromStdString(code.c_str) + ".json");
}
#endif






}
}
