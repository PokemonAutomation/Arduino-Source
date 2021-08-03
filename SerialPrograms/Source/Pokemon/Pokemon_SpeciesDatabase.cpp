/*  Pokemon Species Token
 *
 *  From: https://github.com/PokemonAutomation/Arduino-Source
 *
 */

#include <QtGlobal>
#include "Common/Cpp/Exception.h"
#include "Common/Qt/QtJsonTools.h"
#include "CommonFramework/PersistentSettings.h"
#include "Pokemon_SpeciesDatabase.h"

namespace PokemonAutomation{
namespace Pokemon{


struct SpeciesSlugDatabase{
    SpeciesSlugDatabase();
    static const SpeciesSlugDatabase& instance(){
        static SpeciesSlugDatabase database;
        return database;
    }

    std::vector<std::string> m_national_dex;
    std::map<std::string, SpeciesData> m_slug_to_data;
    std::map<QString, std::string> m_display_name_to_slug;
};

SpeciesSlugDatabase::SpeciesSlugDatabase(){
    QJsonArray slugs = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Pokemon/Pokedex/Pokedex-National.json"
    ).array();
    QJsonObject displays = read_json_file(
        PERSISTENT_SETTINGS().resource_path + "Pokemon/PokemonNameDisplay.json"
    ).object();

    for (const auto& item : slugs){
        QString slug = item.toString();
//        cout << slug.toUtf8().data() << endl;
        if (slug.size() <= 0){
            PA_THROW_StringException("Expected non-empty string for Pokemon slug.");
        }


        SpeciesData data;
        data.m_slug = slug.toUtf8().data();
        m_national_dex.emplace_back(data.m_slug);


        //  Build translations database.
        auto iter0 = displays.find(slug);
        if (iter0 == displays.end()){
            PA_THROW_StringException("No display name found for: " + slug);
        }
        QJsonObject names = iter0.value().toObject();
        for (auto iter1 = names.begin(); iter1 != names.end(); ++iter1){
            data.m_display_names[language_code_to_enum(iter1.key().toUtf8().data())] = iter1.value().toString();
        }


        //  Display name for English.
        auto iter2 = data.m_display_names.find(Language::English);
        if (iter2 == data.m_display_names.end()){
            PA_THROW_StringException("Missing English translation for: " + slug);
        }
        data.m_display_name = iter2->second;


        m_display_name_to_slug.emplace(
            data.m_display_name,
            data.m_slug
        );
        m_slug_to_data.emplace(
            data.m_slug,
            std::move(data)
        );
    }
}


const QString& SpeciesData::display_name(Language language) const{
    auto iter = m_display_names.find(language);
    if (iter == m_display_names.end()){
        PA_THROW_StringException("No data loaded for this language.");
    }
    return iter->second;
}


const std::vector<std::string>& NATIONAL_DEX_SLUGS(){
    return SpeciesSlugDatabase::instance().m_national_dex;
}
const std::map<std::string, SpeciesData>& ALL_POKEMON(){
    return SpeciesSlugDatabase::instance().m_slug_to_data;
}

const SpeciesData& species_slug_to_data(const std::string& slug){
    const std::map<std::string, SpeciesData>& database = SpeciesSlugDatabase::instance().m_slug_to_data;
    auto iter = database.find(slug);
    if (iter == database.end()){
        PA_THROW_StringException("Pokemon slug not found in database: " + slug);
    }
    return iter->second;
}
const std::string& species_display_name_to_slug(const QString& display_name){
    const std::map<QString, std::string>& database = SpeciesSlugDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        PA_THROW_StringException("Display name not found in database: " + display_name);
    }
    return iter->second;
}

const std::string NULL_SLUG;
const std::string& species_display_name_to_slug_nothrow(const QString& display_name){
    const std::map<QString, std::string>& database = SpeciesSlugDatabase::instance().m_display_name_to_slug;
    auto iter = database.find(display_name);
    if (iter == database.end()){
        return NULL_SLUG;
    }
    return iter->second;
}


}
}
