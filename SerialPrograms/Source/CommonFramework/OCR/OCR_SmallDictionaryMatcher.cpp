/*  Small Database Matcher
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/Globals.h"
#include "OCR_StringNormalization.h"
#include "OCR_TextMatcher.h"
#include "OCR_SmallDictionaryMatcher.h"

namespace PokemonAutomation{
namespace OCR{



SmallDictionaryMatcher::SmallDictionaryMatcher(const QString& json_offset, bool first_only)
    : SmallDictionaryMatcher(
        read_json_file(RESOURCE_PATH() + json_offset).object(),
        first_only
    )
{}
SmallDictionaryMatcher::SmallDictionaryMatcher(const QJsonObject& json, bool first_only){
    for (auto iter = json.begin(); iter != json.end(); ++iter){
        Language language = language_code_to_enum(iter.key().toUtf8().data());
        const LanguageData& data = language_data(language);
        m_database.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(language),
            std::forward_as_tuple(iter->toObject(), nullptr, data.random_match_chance, first_only)
        );
        m_languages += language;
    }
}


void SmallDictionaryMatcher::save(const QString& json_path) const{
    QJsonObject root;
    for (const auto& item : m_database){
        root.insert(
            QString::fromStdString(language_data(item.first).code),
            item.second.to_json()
        );
    }
    write_json_file(json_path, QJsonDocument(root));
}



}
}
