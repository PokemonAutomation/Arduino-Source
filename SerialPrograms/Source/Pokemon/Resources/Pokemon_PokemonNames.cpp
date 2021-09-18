/*  Pokemon Names
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon_PokemonNames.h"

namespace PokemonAutomation{
namespace Pokemon{


struct PokemonNameDatabase{
    PokemonNameDatabase();
    static const PokemonNameDatabase& instance(){
        static PokemonNameDatabase database;
        return database;
    }

    static const std::string NULL_SLUG;
    std::map<std::string, PokemonNames> m_slug_to_data;
    std::map<QString, std::string> m_display_name_to_slug;
};
const std::string PokemonNameDatabase::NULL_SLUG;

PokemonNameDatabase::PokemonNameDatabase(){
    QJsonObject displays = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Pokemon/PokemonNameDisplay.json"
    ).object();

    for (auto iter = displays.begin(); iter != displays.end(); ++iter){
        QString slug_qstr = iter.key();
        if (slug_qstr.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }

        std::string slug = slug_qstr.toStdString();

        QJsonObject names = iter.value().toObject();
        PokemonNames data;
        for (auto iter1 = names.begin(); iter1 != names.end(); ++iter1){
            data.m_display_names[language_code_to_enum(iter1.key().toUtf8().data())] = iter1.value().toString();
        }


        //  Display name for English.
        auto iter2 = data.m_display_names.find(Language::English);
        if (iter2 == data.m_display_names.end()){
            PA_THROW_StringException("Missing English translation for: " + slug_qstr);
        }
        data.m_display_name = iter2->second;


        m_display_name_to_slug.emplace(
            data.m_display_name,
            slug
        );
        m_slug_to_data.emplace(
            slug,
            std::move(data)
        );
    }
}


const QString& PokemonNames::display_name(Language language) const{
    auto iter = m_display_names.find(language);
    if (iter == m_display_names.end()){
        PA_THROW_StringException("No data loaded for this language.");
    }
    return iter->second;
}



const PokemonNames& get_pokemon_name(const std::string& slug){
    const std::map<std::string, PokemonNames>& database = PokemonNameDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokemon slug not found in database: " + slug);
    }
    return iter->second;
}
const PokemonNames* get_pokemon_name_nothrow(const std::string& slug){
    const std::map<std::string, PokemonNames>& database = PokemonNameDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        return nullptr;
    }
    return &iter->second;
}
const std::string& parse_pokemon_name(const QString& display_name){
    const std::map<QString, std::string>& database = PokemonNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        PA_THROW_StringException("Display name not found in database: " + display_name);
    }
    return iter->second;
}
const std::string& parse_pokemon_name_nothrow(const QString& display_name){
    const std::map<QString, std::string>& database = PokemonNameDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return PokemonNameDatabase::NULL_SLUG;
    }
    return iter->second;
}


}
}
